#include "engine/buffer.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace engine {
Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage_flags,
               VkMemoryPropertyFlags memory_property_flags)
    : device_{device}, size_{size} {
  Create(size, usage_flags, memory_property_flags);
}

Buffer::~Buffer() {
  Unmap();
  vkDestroyBuffer(device_.GetHandle(), buffer_, nullptr);
  vkFreeMemory(device_.GetHandle(), buffer_memory_, nullptr);
}

VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset) {
  return vkMapMemory(device_.GetHandle(), buffer_memory_, offset, size, 0, &mapped_);
}

void Buffer::Unmap() {
  if (mapped_) {
    vkUnmapMemory(device_.GetHandle(), buffer_memory_);
    mapped_ = nullptr;
  }
}

void Buffer::Write(const void* data, VkDeviceSize size, VkDeviceSize offset) {
  if (size == VK_WHOLE_SIZE) {
    std::memcpy(mapped_, data, size_);
  } else {
    std::memcpy(static_cast<uint8_t*>(mapped_) + offset, data, size);
  }
}

void Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mapped_memory_range{};
  mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mapped_memory_range.memory = buffer_memory_;
  mapped_memory_range.offset = offset;
  mapped_memory_range.size = size;
  vkFlushMappedMemoryRanges(device_.GetHandle(), 1, &mapped_memory_range);
}

void Buffer::CopyTo(const Buffer& dst, VkDeviceSize size, VkDeviceSize src_offset, VkDeviceSize dst_offset) {
  VkCommandBuffer command_buffer = device_.BeginSingleTimeCommands();

  VkBufferCopy copy_region{};
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size = size;
  vkCmdCopyBuffer(command_buffer, buffer_, dst.GetHandle(), 1, &copy_region);

  device_.EndSingleTimeCommands(command_buffer);
}

void Buffer::Create(VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags) {
  VkBufferCreateInfo buffer_create_info{};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.size = size;
  buffer_create_info.usage = usage_flags;
  buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device_.GetHandle(), &buffer_create_info, nullptr, &buffer_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create buffer!"};
  }

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(device_.GetHandle(), buffer_, &memory_requirements);

  VkMemoryAllocateInfo memory_allocate_info{};
  memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memory_allocate_info.allocationSize = memory_requirements.size;
  memory_allocate_info.memoryTypeIndex =
      device_.QueryMemoryType(memory_requirements.memoryTypeBits, memory_property_flags);

  if (vkAllocateMemory(device_.GetHandle(), &memory_allocate_info, nullptr, &buffer_memory_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to allocate buffer memory!"};
  }

  vkBindBufferMemory(device_.GetHandle(), buffer_, buffer_memory_, 0);
}

}  // namespace engine
