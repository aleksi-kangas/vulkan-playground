#pragma once

#include <array>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "engine/device.h"

namespace engine {
class Swapchain {
 public:
  static constexpr uint32_t kMaxFramesInFlight = 2;

  Swapchain(Device& device, VkExtent2D window_extent, std::unique_ptr<Swapchain> old_swapchain = nullptr);
  ~Swapchain();

  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;

  [[nodiscard]] float GetAspectRatio() const {
    return static_cast<float>(swapchain_extent_.width) / static_cast<float>(swapchain_extent_.height);
  }
  [[nodiscard]] VkExtent2D GetExtent() const { return swapchain_extent_; }
  [[nodiscard]] VkFramebuffer GetFramebuffer(uint32_t image_index) const { return framebuffers_[image_index]; }
  [[nodiscard]] VkRenderPass GetRenderPass() const { return render_pass_; }

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

  uint32_t frame_index_ = 0;  // [0, kMaxFramesInFlight)

  std::array<VkSemaphore, kMaxFramesInFlight> image_available_semaphores_{};
  std::array<VkSemaphore, kMaxFramesInFlight> render_finished_semaphores_{};
  std::array<VkFence, kMaxFramesInFlight> in_flight_fences_{};

  [[nodiscard]] VkSemaphore* GetImageAvailableSemaphore() { return &image_available_semaphores_[frame_index_]; }
  [[nodiscard]] VkSemaphore* GetRenderFinishedSemaphore() { return &render_finished_semaphores_[frame_index_]; }
  [[nodiscard]] VkFence* GetInFlightFence() { return &in_flight_fences_[frame_index_]; }

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