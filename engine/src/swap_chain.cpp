#include "engine/swap_chain.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace engine {
Swapchain::Swapchain(Device& device, VkExtent2D window_extent, std::unique_ptr<Swapchain> old_swapchain)
    : device_{device}, window_extent_{window_extent} {
  CreateSwapChain(std::move(old_swapchain));
  CreateImageViews();
  CreateRenderPass();
  CreateFrameBuffers();
  CreateSynchronizationObjects();
}

Swapchain::~Swapchain() {
  for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
    vkDestroySemaphore(device_.GetHandle(), image_available_semaphores_[i], nullptr);
    vkDestroySemaphore(device_.GetHandle(), render_finished_semaphores_[i], nullptr);
    vkDestroyFence(device_.GetHandle(), in_flight_fences_[i], nullptr);
  }

  for (auto framebuffer : framebuffers_) {
    vkDestroyFramebuffer(device_.GetHandle(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(device_.GetHandle(), render_pass_, nullptr);

  for (auto image_view : image_views_) {
    vkDestroyImageView(device_.GetHandle(), image_view, nullptr);
  }

  vkDestroySwapchainKHR(device_.GetHandle(), swapchain_, nullptr);
}

VkResult Swapchain::AcquireNextImage(uint32_t* image_index) {
  vkWaitForFences(device_.GetHandle(), 1, GetInFlightFence(), VK_TRUE, UINT64_MAX);
  vkResetFences(device_.GetHandle(), 1, GetInFlightFence());
  return vkAcquireNextImageKHR(device_.GetHandle(), swapchain_, UINT64_MAX, *GetImageAvailableSemaphore(),
                               VK_NULL_HANDLE, image_index);
}

VkResult Swapchain::SubmitAndPresent(const VkCommandBuffer& command_buffer, uint32_t image_index) {
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Wait until the image is available...
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = GetImageAvailableSemaphore();

  // ...at the end of the pipeline.
  std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.pWaitDstStageMask = wait_stages.data();

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  // Signal that the image is ready to be presented.
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = GetRenderFinishedSemaphore();

  if (vkQueueSubmit(device_.GetGraphicsQueue(), 1, &submit_info, *GetInFlightFence()) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to submit command buffer!"};
  }

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // Wait until the image is ready to be presented.
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = GetRenderFinishedSemaphore();

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain_;
  present_info.pImageIndices = &image_index;

  auto present_result = vkQueuePresentKHR(device_.GetPresentQueue(), &present_info);
  frame_index_ = (frame_index_ + 1) % kMaxFramesInFlight;
  return present_result;
}

void Swapchain::CreateSwapChain(std::unique_ptr<Swapchain> old_swapchain) {
  SwapchainSupportDetails swapchain_support_details = device_.QuerySwapchainSupportDetails();

  VkSurfaceFormatKHR surface_format = PickSwapchainSurfaceFormat(swapchain_support_details.formats);
  VkPresentModeKHR present_mode = PickSwapchainPresentMode(swapchain_support_details.present_modes);
  VkExtent2D extent = PickSwapchainExtent(swapchain_support_details.capabilities, window_extent_);

  uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1;
  // Make sure not to exceed the maximum number of images (0 = no maximum)
  if (swapchain_support_details.capabilities.maxImageCount > 0) {
    std::clamp(image_count, image_count, swapchain_support_details.capabilities.maxImageCount);
  }

  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = device_.GetSurface();
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = surface_format.format;
  swapchain_info.imageColorSpace = surface_format.colorSpace;
  swapchain_info.imageExtent = extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  const uint32_t graphics_queue_family_index = device_.GetGraphicsQueueFamilyIndex();
  const uint32_t present_queue_family_index = device_.GetPresentQueueFamilyIndex();
  std::array<uint32_t, 2> queue_family_indices = {graphics_queue_family_index, present_queue_family_index};

  if (graphics_queue_family_index != present_queue_family_index) {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
    swapchain_info.pQueueFamilyIndices = queue_family_indices.data();
  } else {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount = 0;
    swapchain_info.pQueueFamilyIndices = nullptr;
  }

  swapchain_info.preTransform = swapchain_support_details.capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = present_mode;
  swapchain_info.clipped = VK_TRUE;
  swapchain_info.oldSwapchain = old_swapchain ? old_swapchain->swapchain_ : VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device_.GetHandle(), &swapchain_info, nullptr, &swapchain_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create swap chain!"};
  }

  if (vkGetSwapchainImagesKHR(device_.GetHandle(), swapchain_, &image_count, nullptr) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to get swap chain images!"};
  }
  images_.resize(image_count);
  if (vkGetSwapchainImagesKHR(device_.GetHandle(), swapchain_, &image_count, images_.data()) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to get swap chain images!"};
  }

  image_format_ = surface_format.format;
  swapchain_extent_ = extent;
}

void Swapchain::CreateRenderPass() {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = image_format_;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  if (vkCreateRenderPass(device_.GetHandle(), &render_pass_info, nullptr, &render_pass_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create render pass!"};
  }
}

void Swapchain::CreateImageViews() {
  image_views_.resize(images_.size());

  for (size_t i = 0; i < images_.size(); ++i) {
    VkImageViewCreateInfo image_view_info{};
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = images_[i];
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = image_format_;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device_.GetHandle(), &image_view_info, nullptr, &image_views_[i]) != VK_SUCCESS) {
      throw std::runtime_error{"Failed to create image view!"};
    }
  }
}

void Swapchain::CreateFrameBuffers() {
  framebuffers_.resize(image_views_.size());

  for (size_t i = 0; i < image_views_.size(); ++i) {
    std::array<VkImageView, 1> attachments = {image_views_[i]};

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass_;
    framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = swapchain_extent_.width;
    framebuffer_info.height = swapchain_extent_.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(device_.GetHandle(), &framebuffer_info, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
      throw std::runtime_error{"Failed to create framebuffer!"};
    }
  }
}

void Swapchain::CreateSynchronizationObjects() {
  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
    if (vkCreateSemaphore(device_.GetHandle(), &semaphore_info, nullptr, &image_available_semaphores_[i]) !=
            VK_SUCCESS ||
        vkCreateSemaphore(device_.GetHandle(), &semaphore_info, nullptr, &render_finished_semaphores_[i]) !=
            VK_SUCCESS ||
        vkCreateFence(device_.GetHandle(), &fence_info, nullptr, &in_flight_fences_[i]) != VK_SUCCESS) {
      throw std::runtime_error{"Failed to create synchronization objects!"};
    }
  }
}

VkSurfaceFormatKHR Swapchain::PickSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
  auto it =
      std::find_if(available_formats.begin(), available_formats.end(), [](const VkSurfaceFormatKHR& available_format) {
        return available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
               available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });
  return it != available_formats.end() ? *it : available_formats[0];
}

VkPresentModeKHR Swapchain::PickSwapchainPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
  auto it = std::find_if(available_present_modes.begin(), available_present_modes.end(),
                         [](const VkPresentModeKHR& available_present_mode) {
                           return available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR;
                         });
  return it != available_present_modes.end() ? *it : VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::PickSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D window_extent) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actual_extent = window_extent;
    actual_extent.width =
        std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height =
        std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual_extent;
  }
}

}  // namespace engine
