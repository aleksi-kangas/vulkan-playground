#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

#include "engine/window.h"

namespace engine {
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class Device {
 public:
  explicit Device(Window& window);
  ~Device();

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  [[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const { return graphics_queue_family_index_; }
  [[nodiscard]] uint32_t GetPresentQueueFamilyIndex() const { return present_queue_family_index_; }
  [[nodiscard]] SwapChainSupportDetails QuerySwapChainSupportDetails() const {
    return QuerySwapChainSupportDetails(physical_device_, surface_);
  }

 private:
  Window& window_;

  VkInstance instance_ = VK_NULL_HANDLE;
#ifdef ENABLE_VALIDATION_LAYERS
  VkDebugUtilsMessengerEXT debug_utils_messenger_ = VK_NULL_HANDLE;
#endif
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physical_device_properties_{};

  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue graphics_queue_ = VK_NULL_HANDLE;
  uint32_t graphics_queue_family_index_ = 0;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  uint32_t present_queue_family_index_ = 0;

  void CreateInstance();
#ifdef ENABLE_VALIDATION_LAYERS
  void CreateDebugUtilsMessenger();
  static bool CheckValidationLayerSupport();
#endif
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();

  int32_t RatePhysicalDeviceSuitability(VkPhysicalDevice physical_device);
  static bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice physical_device);

  static SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

#ifdef ENABLE_VALIDATION_LAYERS
  static constexpr std::array<const char*, 1> kValidationLayers = {"VK_LAYER_KHRONOS_validation"};
#endif

#ifdef __APPLE__
  static constexpr std::array<const char*, 2> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                   "VK_KHR_portability_subset"};
#else
  static constexpr std::array<const char*, 1> kDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
#endif
};
}  // namespace engine