#pragma once

#include "engine/device.h"

namespace engine {
class Buffer {
 public:
  Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage_flags,
         VkMemoryPropertyFlags memory_property_flags);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  [[nodiscard]] VkBuffer GetHandle() const { return buffer_; }

  VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void Unmap();

  void Write(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

  void CopyTo(const Buffer& dst, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);

 private:
  Device& device_;

  VkDeviceSize size_ = 0;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory buffer_memory_ = VK_NULL_HANDLE;

  void* mapped_ = nullptr;

  void Create(VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags);
};
}  // namespace engine
