#include "engine/renderer.h"

#include <cassert>
#include <utility>

namespace engine {
Renderer::Renderer(Window& window, Device& device) : window_{window}, device_{device} {
  swap_chain_ = std::make_unique<Swapchain>(device_, window_.GetExtent());
  AllocateCommandBuffers();
}

Renderer::~Renderer() {
  FreeCommandBuffers();
}

VkCommandBuffer Renderer::BeginFrame() {
  auto image_acquire_result = swap_chain_->AcquireNextImage(&image_index_);
  if (image_acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
    // Should we recreate swapchain if VK_SUBOPTIMAL_KHR is returned? For now, ignore.
    RecreateSwapchain();
    return VK_NULL_HANDLE;
  } else if (image_acquire_result != VK_SUCCESS && image_acquire_result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error{"Failed to acquire swap chain image!"};
  }

  auto command_buffer = GetCurrentCommandBuffer();
  VkCommandBufferBeginInfo command_buffer_begin_info{};
  command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to begin recording command buffer!"};
  }
  return command_buffer;
}

void Renderer::BeginRenderPass(VkCommandBuffer command_buffer) const {
  // Dynamic viewport and scissor rectangles.
  // Important to use swap chain extent, not window extent (e.g. Apple Retina).
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swap_chain_->GetExtent().width);
  viewport.height = static_cast<float>(swap_chain_->GetExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_->GetExtent();
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  VkRenderPassBeginInfo render_pass_begin_info{};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = swap_chain_->GetRenderPass();
  render_pass_begin_info.framebuffer = swap_chain_->GetFramebuffer(image_index_);
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent = swap_chain_->GetExtent();

  VkClearValue clear_color{
      .color = {{0.0f, 0.0f, 0.0f, 1.0f}},
  };
  render_pass_begin_info.clearValueCount = 1;
  render_pass_begin_info.pClearValues = &clear_color;

  vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::EndRenderPass(VkCommandBuffer command_buffer) const {
  vkCmdEndRenderPass(command_buffer);
}

void Renderer::EndFrame() {
  auto command_buffer = GetCurrentCommandBuffer();
  if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to record command buffer!"};
  }
  auto submit_and_present_result = swap_chain_->SubmitAndPresent(command_buffer, image_index_);
  if (submit_and_present_result == VK_ERROR_OUT_OF_DATE_KHR || submit_and_present_result == VK_SUBOPTIMAL_KHR ||
      window_.HasResized()) {
    window_.ResetResizedFlag();
    RecreateSwapchain();
  } else if (submit_and_present_result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to submit command buffer and present swap chain image!"};
  }

  frame_index_ = (frame_index_ + 1) % Swapchain::kMaxFramesInFlight;
}

void Renderer::AllocateCommandBuffers() {
  VkCommandBufferAllocateInfo command_buffer_allocate_info{};
  command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_allocate_info.commandPool = device_.GetGraphicsCommandPool();
  command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_allocate_info.commandBufferCount = Swapchain::kMaxFramesInFlight;

  if (vkAllocateCommandBuffers(device_.GetHandle(), &command_buffer_allocate_info, command_buffers_.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error{"Failed to allocate command buffers!"};
  }
}

void Renderer::FreeCommandBuffers() {
  vkFreeCommandBuffers(device_.GetHandle(), device_.GetGraphicsCommandPool(), Swapchain::kMaxFramesInFlight,
                       command_buffers_.data());
}

void Renderer::RecreateSwapchain() {
  assert(swap_chain_);
  auto extent = window_.GetExtent();
  while (extent.width == 0 || extent.height == 0) {  // Minimized
    extent = window_.GetExtent();
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(device_.GetHandle());

  if (swap_chain_) {
    auto old_swap_chain = std::move(swap_chain_);
    swap_chain_ = std::make_unique<Swapchain>(device_, extent, std::move(old_swap_chain));
  }
}

}  // namespace engine
