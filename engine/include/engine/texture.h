#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "engine/device.h"

namespace engine {
class TextureManager;

class Texture {
 public:
  static Texture* CreateFromFile(TextureManager& manager, const std::filesystem::path& file_path);

  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  [[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const {
    return {.sampler = sampler_, .imageView = image_view_, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  void Bind(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

 private:
  Texture(Device& device, const std::filesystem::path& file_path);

  Device& device_;

  VkImage image_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkImageView image_view_ = VK_NULL_HANDLE;
  VkSampler sampler_ = VK_NULL_HANDLE;

  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  void CreateImage(const std::vector<uint8_t>& bytes, uint32_t width, uint32_t height);
  void CreateImageView();
  void CreateSampler();

  void TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
};

class TextureManager {
 public:
  explicit TextureManager(Device& device) : device_{device} {}

  TextureManager(const TextureManager&) = delete;
  TextureManager& operator=(const TextureManager&) = delete;

  Texture* Add(const std::string& name, std::unique_ptr<Texture> texture) {
    auto [it, success] = textures_.try_emplace(name, std::move(texture));
    return it->second.get();
  }
  [[nodiscard]] Texture* Get(const std::string& name) const {
    auto it = textures_.find(name);
    if (it == textures_.end()) {
      return nullptr;
    }
    return it->second.get();
  }

 private:
  Device& device_;

  std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;

  friend class Texture;
};

}  // namespace engine
