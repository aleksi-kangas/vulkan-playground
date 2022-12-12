#include "engine/textures/texture.h"

#include "engine/buffer.h"
#include "engine/utils.h"

namespace engine::textures {
Texture::Texture(Device& device, const std::filesystem::path& file_path) : device_{device} {
  uint32_t width, height, channels;
  const std::vector<uint8_t> image_bytes = utils::ReadImage(file_path, width, height, channels);
  CreateImage(image_bytes, width, height);
  CreateImageView();
  CreateSampler();

  // TODO For now, hardcode this here...
  VkDescriptorSetLayoutBinding layout_binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };
  VkDescriptorSetLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &layout_binding,
  };
  if (vkCreateDescriptorSetLayout(device_.GetHandle(), &layout_info, nullptr, &descriptor_set_layout_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create descriptor set layout!"};
  }

  VkDescriptorPoolSize pool_size = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
  };
  VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
  };
  if (vkCreateDescriptorPool(device_.GetHandle(), &pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create descriptor pool!"};
  }

  VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool_,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_set_layout_,
  };
  if (vkAllocateDescriptorSets(device_.GetHandle(), &alloc_info, &descriptor_set_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to allocate descriptor set!"};
  }

  auto image_info = GetDescriptorInfo();
  VkWriteDescriptorSet descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set_,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
  };
  vkUpdateDescriptorSets(device_.GetHandle(), 1, &descriptor_write, 0, nullptr);
}

Texture::~Texture() {
  vkDestroySampler(device_.GetHandle(), sampler_, nullptr);
  vkDestroyImageView(device_.GetHandle(), image_view_, nullptr);
  vkDestroyImage(device_.GetHandle(), image_, nullptr);
  vkFreeMemory(device_.GetHandle(), memory_, nullptr);
}

void Texture::Bind(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout) {
  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1, &descriptor_set_, 0,
                          nullptr);
}

void Texture::CreateImage(const std::vector<uint8_t>& bytes, uint32_t width, uint32_t height) {
  Buffer staging_buffer{device_, bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
  staging_buffer.Map();
  staging_buffer.Write(reinterpret_cast<const void*>(bytes.data()));

  VkImageCreateInfo image_info{};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateImage(device_.GetHandle(), &image_info, nullptr, &image_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create image!"};
  }

  VkMemoryRequirements memory_requirements{};
  vkGetImageMemoryRequirements(device_.GetHandle(), image_, &memory_requirements);

  VkMemoryAllocateInfo memory_allocate_info{};
  memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memory_allocate_info.allocationSize = memory_requirements.size;
  memory_allocate_info.memoryTypeIndex =
      device_.QueryMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (vkAllocateMemory(device_.GetHandle(), &memory_allocate_info, nullptr, &memory_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to allocate image memory!"};
  }

  if (vkBindImageMemory(device_.GetHandle(), image_, memory_, 0) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to bind image memory!"};
  }

  TransitionImageLayout(image_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  VkCommandBuffer command_buffer = device_.BeginSingleTimeCommands();
  vkCmdCopyBufferToImage(command_buffer, staging_buffer.GetHandle(), image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                         &region);
  device_.EndSingleTimeCommands(command_buffer);

  TransitionImageLayout(image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Texture::CreateImageView() {
  VkImageViewCreateInfo view_info{};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image_;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;
  if (vkCreateImageView(device_.GetHandle(), &view_info, nullptr, &image_view_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create texture image view!"};
  }
}

void Texture::CreateSampler() {
  VkSamplerCreateInfo sampler_info{};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_FALSE;
  sampler_info.maxAnisotropy = 16;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;
  if (vkCreateSampler(device_.GetHandle(), &sampler_info, nullptr, &sampler_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create texture sampler!"};
  }
}

void Texture::TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {
  VkCommandBuffer command_buffer = device_.BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument{"Unsupported layout transition!"};
  }

  vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  device_.EndSingleTimeCommands(command_buffer);
}

}  // namespace engine::textures
