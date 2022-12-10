#include "engine/renderer.h"

namespace engine {
Renderer::Renderer(Window& window, Device& device) : window_{window}, device_{device} {
  swap_chain_ = std::make_unique<Swapchain>(device_, window_.GetExtent());
  AllocateCommandBuffers();
}

Renderer::~Renderer() {
  FreeCommandBuffers();
}

void Renderer::AllocateCommandBuffers() {
  VkCommandBufferAllocateInfo command_buffer_allocate_info{};
  command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_allocate_info.commandPool = device_.GetGraphicsCommandPool();
  command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_allocate_info.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(device_.GetHandle(), &command_buffer_allocate_info, &command_buffer_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to allocate command buffers!"};
  }
}

void Renderer::FreeCommandBuffers() {
  vkFreeCommandBuffers(device_.GetHandle(), device_.GetGraphicsCommandPool(), 1, &command_buffer_);
}

}  // namespace engine
