#include "engine/device.h"

#include <algorithm>
#include <cassert>
#ifdef ENABLE_VALIDATION_LAYERS
#include <iostream>
#endif
#include <map>
#include <optional>
#include <stdexcept>
#include <unordered_set>

namespace {
#ifdef ENABLE_VALIDATION_LAYERS
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /* message_severity */,
                                             VkDebugUtilsMessageTypeFlagsEXT /* message_type */,
                                             const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                             void* /* user_data */) {
  std::cerr << "Validation Layer: " << callback_data->pMessage << std::endl;
  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                      const VkAllocationCallbacks* allocator,
                                      VkDebugUtilsMessengerEXT* debug_messenger) {
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    return func(instance, create_info, allocator, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
                                   const VkAllocationCallbacks* allocator) {
  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    func(instance, debug_messenger, allocator);
  }
}
#endif

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  [[nodiscard]] bool IsComplete() const { return graphics_family.has_value() && present_family.has_value(); }
};

QueueFamilyIndices QueryQueueFamilyIndices(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

  uint32_t i = 0;
  for (const auto& queue_family : queue_families) {
    if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
    if (queue_family.queueCount > 0 && present_support) {
      indices.present_family = i;
    }
    if (indices.IsComplete()) {
      break;
    }
    i++;
  }
  assert(indices.IsComplete());
  return indices;
}

std::vector<const char*> RequiredExtensions() {
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  std::vector<const char*> required_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

#ifdef ENABLE_VALIDATION_LAYERS
  required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#ifdef __APPLE__
  required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  required_extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

  return required_extensions;
}

void VerifyRequiredExtensions() {
  uint32_t extension_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> extensions(extension_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

#ifdef ENABLE_VALIDATION_LAYERS
  std::cout << "Available extensions:" << std::endl;
#endif
  std::unordered_set<std::string> available_extensions;
  for (const auto& extension : extensions) {
#ifdef ENABLE_VALIDATION_LAYERS
    std::cout << "\t" << extension.extensionName << std::endl;
#endif
    available_extensions.insert(extension.extensionName);
  }

#ifdef ENABLE_VALIDATION_LAYERS
  std::cout << "Required extensions:" << std::endl;
#endif
  auto required_extensions = RequiredExtensions();
  for (const auto& extension : required_extensions) {
#ifdef ENABLE_VALIDATION_LAYERS
    std::cout << "\t" << extension << std::endl;
#endif
    if (!available_extensions.contains(extension)) {
      throw std::runtime_error("Required extension not available!");
    }
  }
}

}  // namespace

namespace engine {
Device::Device(Window& window) : window_{window} {
  CreateInstance();
#ifdef ENABLE_VALIDATION_LAYERS
  CreateDebugUtilsMessenger();
#endif
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateGraphicsCommandPool();
}

Device::~Device() {
  vkDestroyCommandPool(device_, graphics_command_pool_, nullptr);

  vkDestroyDevice(device_, nullptr);

  vkDestroySurfaceKHR(instance_, surface_, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
  DestroyDebugUtilsMessengerEXT(instance_, debug_utils_messenger_, nullptr);
#endif

  vkDestroyInstance(instance_, nullptr);
}

uint32_t Device::QueryMemoryType(uint32_t type_filter, VkMemoryPropertyFlags memory_property_flags) const {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) &&
        (memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags) {
      return i;
    }
  }

  throw std::runtime_error{"Failed to find suitable memory type!"};
}

void Device::CreateInstance() {
#ifdef ENABLE_VALIDATION_LAYERS
  if (!CheckValidationLayerSupport()) {
    throw std::runtime_error{"Vulkan validation layers requested, but not available!"};
  }
#endif

  VkApplicationInfo application_info{};
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pApplicationName = "Vulkan Playground";
  application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  application_info.pEngineName = "Vulkan Engine";
  application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  application_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &application_info;
#ifdef __APPLE__
  instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  VerifyRequiredExtensions();
  auto required_extensions = RequiredExtensions();
  instance_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
  instance_info.ppEnabledExtensionNames = required_extensions.data();

#ifdef ENABLE_VALIDATION_LAYERS
  instance_info.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
  instance_info.ppEnabledLayerNames = kValidationLayers.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_info{};
  debug_utils_messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_utils_messenger_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_utils_messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_utils_messenger_info.pfnUserCallback = DebugCallback;
  instance_info.pNext = &debug_utils_messenger_info;
#else
  instance_info.enabledLayerCount = 0;
#endif

  if (vkCreateInstance(&instance_info, nullptr, &instance_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create VkInstance!"};
  }
}

#ifdef ENABLE_VALIDATION_LAYERS
void Device::CreateDebugUtilsMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_info{};
  debug_utils_messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_utils_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_utils_messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_utils_messenger_info.pfnUserCallback = DebugCallback;

  if (CreateDebugUtilsMessengerEXT(instance_, &debug_utils_messenger_info, nullptr, &debug_utils_messenger_) !=
      VK_SUCCESS) {
    throw std::runtime_error{"Failed to create VkDebugUtilsMessengerEXT!"};
  }
}
#endif

#ifdef ENABLE_VALIDATION_LAYERS
bool Device::CheckValidationLayerSupport() {
  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : kValidationLayers) {
    auto it = std::find_if(available_layers.begin(), available_layers.end(),
                           [layer_name](const VkLayerProperties& layer_properties) {
                             return strcmp(layer_name, layer_properties.layerName) == 0;
                           });
    if (it == available_layers.end()) {
      return false;
    }
  }
  return true;
}
#endif

void Device::CreateSurface() {
  window_.CreateSurface(instance_, &surface_);
}

void Device::PickPhysicalDevice() {
  uint32_t physical_device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr);
  if (physical_device_count == 0) {
    throw std::runtime_error{"Failed to find GPUs with Vulkan support!"};
  }

  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());

  std::multimap<int32_t, VkPhysicalDevice> candidates;
  for (const auto& physical_device : physical_devices) {
    int32_t score = RatePhysicalDeviceSuitability(physical_device);
    candidates.emplace(score, physical_device);
  }

  if (candidates.rbegin()->first > 0) {
    physical_device_ = candidates.rbegin()->second;
    vkGetPhysicalDeviceProperties(physical_device_, &physical_device_properties_);
  } else {
    throw std::runtime_error{"Failed to find a suitable GPU!"};
  }

#ifdef ENABLE_VALIDATION_LAYERS
  VkPhysicalDeviceProperties physical_device_properties;
  vkGetPhysicalDeviceProperties(physical_device_, &physical_device_properties);
  std::cout << "GPU: " << physical_device_properties.deviceName << std::endl;
#endif
}

void Device::CreateLogicalDevice() {
  QueueFamilyIndices queue_family_indices = QueryQueueFamilyIndices(physical_device_, surface_);
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::unordered_set<uint32_t> unique_queue_family_indices = {queue_family_indices.graphics_family.value(),
                                                              queue_family_indices.present_family.value()};

  constexpr float kQueuePriority = 1.0f;
  for (uint32_t queue_family_index : unique_queue_family_indices) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &kQueuePriority;
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};
  //  device_features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();

  create_info.pEnabledFeatures = &device_features;

  create_info.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
  create_info.ppEnabledExtensionNames = kDeviceExtensions.data();

#ifdef ENABLE_VALIDATION_LAYERS
  create_info.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
  create_info.ppEnabledLayerNames = kValidationLayers.data();
#else
  create_info.enabledLayerCount = 0;
#endif

  if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device!");
  }

  vkGetDeviceQueue(device_, queue_family_indices.graphics_family.value(), 0, &graphics_queue_);
  graphics_queue_family_index_ = queue_family_indices.graphics_family.value();
  vkGetDeviceQueue(device_, queue_family_indices.present_family.value(), 0, &present_queue_);
  present_queue_family_index_ = queue_family_indices.present_family.value();
}

void Device::CreateGraphicsCommandPool() {
  QueueFamilyIndices queue_family_indices = QueryQueueFamilyIndices(physical_device_, surface_);

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
  pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device_, &pool_info, nullptr, &graphics_command_pool_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool!");
  }
}

int32_t Device::RatePhysicalDeviceSuitability(VkPhysicalDevice physical_device) {
  VkPhysicalDeviceProperties physical_device_properties;
  vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
  VkPhysicalDeviceFeatures physical_device_features;
  vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

  int32_t score = 0;
  if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }
  if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    score += 100;
  }
  if (!QueryQueueFamilyIndices(physical_device, surface_).IsComplete()) {
    score = -1;
  }
  if (!CheckPhysicalDeviceExtensionSupport(physical_device)) {
    score = -1;
  }
  if (!physical_device_features.samplerAnisotropy) {
    score = -1;
  }
  SwapchainSupportDetails swap_chain_support_details = QuerySwapchainSupportDetails(physical_device, surface_);
  if (swap_chain_support_details.formats.empty() || swap_chain_support_details.present_modes.empty()) {
    score = -1;
  }
  return score > 0 ? score : -1;
}

bool Device::CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice physical_device) {
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

  return std::all_of(kDeviceExtensions.begin(), kDeviceExtensions.end(),
                     [&available_extensions](const char* extension_name) {
                       auto it = std::find_if(available_extensions.begin(), available_extensions.end(),
                                              [extension_name](const VkExtensionProperties& extension_properties) {
                                                return strcmp(extension_name, extension_properties.extensionName) == 0;
                                              });
                       return it != available_extensions.end();
                     });
}

SwapchainSupportDetails Device::QuerySwapchainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  SwapchainSupportDetails details{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                              details.present_modes.data());
  }

  return details;
}

}  // namespace engine