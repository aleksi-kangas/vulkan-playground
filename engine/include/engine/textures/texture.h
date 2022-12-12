#pragma once

#include <cstdint>
#include <filesystem>

#include <vulkan/vulkan.h>

#include "engine/device.h"

namespace engine::textures {
class Texture {
 public:
  Texture(Device& device, const std::filesystem::path& file_path);
  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  [[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const {
    return {.sampler = sampler_, .imageView = image_view_, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  void Bind(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

 private:
  Device& device_;

  VkImage image_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkImageView image_view_ = VK_NULL_HANDLE;
  VkSampler sampler_ = VK_NULL_HANDLE;

  // TODO How to handle these?
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  void CreateImage(const std::vector<uint8_t>& bytes, uint32_t width, uint32_t height);
  void CreateImageView();
  void CreateSampler();

  void TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
};
}  // namespace engine::textures
