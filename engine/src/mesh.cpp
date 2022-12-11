#include "engine/mesh.h"

#include <array>
#include <cassert>

namespace engine {
Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
  CreateVertexBuffer(device, vertices);
  CreateIndexBuffer(device, indices);
}

Mesh::~Mesh() = default;

void Mesh::Bind(VkCommandBuffer command_buffer) const {
  std::array<VkBuffer, 1> vertex_buffers = {vertex_buffer_->GetHandle()};
  std::array<VkDeviceSize, 1> offsets = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, static_cast<uint32_t>(vertex_buffers.size()), vertex_buffers.data(),
                         offsets.data());

  if (index_buffer_) {
    vkCmdBindIndexBuffer(command_buffer, index_buffer_->GetHandle(), 0, VK_INDEX_TYPE_UINT32);
  }
}

void Mesh::Draw(VkCommandBuffer command_buffer) const {
  if (index_buffer_) {
    vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
  } else {
    vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
  }
}

void Mesh::CreateVertexBuffer(Device& device, const std::vector<Vertex>& vertices) {
  assert(!vertices.empty());
  vertex_count_ = static_cast<uint32_t>(vertices.size());

  const VkDeviceSize buffer_size = sizeof(Vertex) * vertex_count_;

  Buffer staging_buffer{device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  staging_buffer.Map();
  staging_buffer.Write(reinterpret_cast<const void*>(vertices.data()));

  vertex_buffer_ = std::make_unique<Buffer>(device, buffer_size,
                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  staging_buffer.CopyTo(*vertex_buffer_, buffer_size);
}

void Mesh::CreateIndexBuffer(Device& device, const std::vector<uint32_t>& indices) {
  index_count_ = static_cast<uint32_t>(indices.size());
  if (index_count_ == 0)
    return;

  const VkDeviceSize buffer_size = sizeof(uint32_t) * index_count_;

  Buffer staging_buffer{device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  staging_buffer.Map();
  staging_buffer.Write(reinterpret_cast<const void*>(indices.data()));

  index_buffer_ =
      std::make_unique<Buffer>(device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  staging_buffer.CopyTo(*index_buffer_, buffer_size);
}

}  // namespace engine
