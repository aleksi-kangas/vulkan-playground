#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

#include "engine/window.h"

namespace engine {
class Device {
 public:
  explicit Device(Window& window);
  ~Device();

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

 private:
  VkInstance instance_ = VK_NULL_HANDLE;
#ifdef ENABLE_VALIDATION_LAYERS
  VkDebugUtilsMessengerEXT debug_utils_messenger_ = VK_NULL_HANDLE;
#endif
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physical_device_properties_{};

  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue graphics_queue_ = VK_NULL_HANDLE;
  uint32_t graphics_queue_family_index_ = 0;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  uint32_t present_queue_family_index_ = 0;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
};
}  // namespace engine