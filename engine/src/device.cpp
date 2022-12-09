#include "engine/device.h"

#include <stdexcept>

#include <VkBootstrap.h>

namespace engine {
Device::Device(Window& window) {
  vkb::InstanceBuilder instance_builder{};
  instance_builder.set_app_name("Vulkan Playground");
  instance_builder.require_api_version(1, 0);
#ifdef ENABLE_VALIDATION_LAYERS
  instance_builder.request_validation_layers(true);
  instance_builder.use_default_debug_messenger();
#endif
  auto vkb_instance = instance_builder.build();
  if (!vkb_instance) {
    throw std::runtime_error{"Failed to create Vulkan instance"};
  }

  instance_ = vkb_instance->instance;
#ifdef ENABLE_VALIDATION_LAYERS
  debug_utils_messenger_ = vkb_instance->debug_messenger;
#endif

  window.CreateSurface(instance_, &surface_);

  vkb::PhysicalDeviceSelector physical_device_selector{vkb_instance.value()};
  physical_device_selector.set_minimum_version(1, 0);
  physical_device_selector.set_surface(surface_);
  physical_device_selector.require_present();
  auto vkb_physical_device = physical_device_selector.select();
  if (!vkb_physical_device) {
    throw std::runtime_error{"Failed to find a suitable GPU"};
  }
  physical_device_ = vkb_physical_device->physical_device;
  physical_device_properties_ = vkb_physical_device->properties;

  vkb::DeviceBuilder device_builder{vkb_physical_device.value()};
  auto vkb_device = device_builder.build();
  if (!vkb_device) {
    throw std::runtime_error{"Failed to create Vulkan device"};
  }
  device_ = vkb_device->device;

  auto vkb_graphics_queue = vkb_device->get_queue(vkb::QueueType::graphics);
  auto vkb_graphics_queue_family_index = vkb_device->get_queue_index(vkb::QueueType::graphics);
  if (!vkb_graphics_queue || !vkb_graphics_queue_family_index) {
    throw std::runtime_error{"Failed to get graphics queue"};
  }
  graphics_queue_ = vkb_graphics_queue.value();
  graphics_queue_family_index_ = vkb_graphics_queue_family_index.value();

  auto vkb_present_queue = vkb_device->get_queue(vkb::QueueType::present);
  auto vkb_present_queue_family_index = vkb_device->get_queue_index(vkb::QueueType::present);
  if (!vkb_present_queue || !vkb_present_queue_family_index) {
    throw std::runtime_error{"Failed to get present queue"};
  }
  present_queue_ = vkb_present_queue.value();
  present_queue_family_index_ = vkb_present_queue_family_index.value();
}

Device::~Device() {
  vkDestroyDevice(device_, nullptr);

  vkDestroySurfaceKHR(instance_, surface_, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
  auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));
  vkDestroyDebugUtilsMessengerEXT(instance_, debug_utils_messenger_, nullptr);
#endif

  vkDestroyInstance(instance_, nullptr);
}

}  // namespace engine