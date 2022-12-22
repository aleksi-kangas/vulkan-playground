#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include <engine/buffer.h>
#include <engine/device.h>
#include <engine/vertex.h>

namespace engine {

class Mesh {
 public:
  Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices = {});
  ~Mesh();

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  static std::unique_ptr<Mesh> CreateSphereMesh(Device& device, uint32_t cube_face_resolution);

  void Bind(VkCommandBuffer command_buffer) const;
  void Draw(VkCommandBuffer command_buffer) const;

 private:
  std::unique_ptr<Buffer> vertex_buffer_;
  uint32_t vertex_count_ = 0;

  std::unique_ptr<Buffer> index_buffer_;
  uint32_t index_count_ = 0;

  void CreateVertexBuffer(Device& device, const std::vector<Vertex>& vertices);
  void CreateIndexBuffer(Device& device, const std::vector<uint32_t>& indices);
};
}  // namespace engine
