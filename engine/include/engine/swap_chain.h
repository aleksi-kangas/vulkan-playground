#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "engine/device.h"

namespace engine {
class Swapchain {
 public:
  Swapchain(Device& device, VkExtent2D window_extent, std::unique_ptr<Swapchain> old_swapchain = nullptr);
  ~Swapchain();

  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;

  VkResult AcquireNextImage(uint32_t* image_index);
  VkResult SubmitAndPresent(const VkCommandBuffer& command_buffer, uint32_t image_index);

 private:
  Device& device_;
  VkExtent2D window_extent_;

  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  VkExtent2D swapchain_extent_{};
  VkFormat image_format_ = VK_FORMAT_UNDEFINED;
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;

  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> framebuffers_;

  VkSemaphore image_available_semaphore_ = VK_NULL_HANDLE;
  VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
  VkFence in_flight_fence_ = VK_NULL_HANDLE;

  void CreateSwapChain(std::unique_ptr<Swapchain> old_swapchain);
  void CreateImageViews();
  void CreateRenderPass();
  void CreateFrameBuffers();
  void CreateSynchronizationObjects();

  static VkSurfaceFormatKHR PickSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
  static VkPresentModeKHR PickSwapchainPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
  static VkExtent2D PickSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D window_extent);
};
}  // namespace engine