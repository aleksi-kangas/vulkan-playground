#include "engine/mesh.h"

#include <array>
#include <cassert>

namespace {
struct CubeFace {
  uint32_t index = 0;
  glm::vec3 origin{};
  glm::vec3 u{};
  glm::vec3 v{};
  glm::vec3 normal{};
};

glm::vec3 CubeToSphere(glm::vec3 cube_point) {
  const glm::vec3 p2 = cube_point * cube_point;
  const float x = cube_point.x * sqrt(1 - (p2.y + p2.z) / 2 + (p2.y * p2.z) / 3);
  const float y = cube_point.y * sqrt(1 - (p2.x + p2.z) / 2 + (p2.x * p2.z) / 3);
  const float z = cube_point.z * sqrt(1 - (p2.x + p2.y) / 2 + (p2.x * p2.y) / 3);
  return {x, y, z};
}

glm::vec2 SphereToUV(glm::vec3 sphere_point) {
  const float u = 0.5f + atan2(sphere_point.z, sphere_point.x) / (2 * glm::pi<float>());
  const float v = 0.5f - asin(sphere_point.y) / glm::pi<float>();
  return {u, -v};
}

void GenerateCubeFace(const CubeFace& cube_face, uint32_t cube_face_resolution, std::vector<engine::Vertex>& vertices,
                      std::vector<uint32_t>& indices) {
  const uint32_t face_vertices_offset = cube_face.index * cube_face_resolution * cube_face_resolution;
  const uint32_t face_indices_offset = cube_face.index * (cube_face_resolution - 1) * (cube_face_resolution - 1) * 6;
  uint32_t indices_index = 0;

  for (uint32_t u = 0; u < cube_face_resolution; ++u) {
    for (uint32_t v = 0; v < cube_face_resolution; ++v) {
      const uint32_t vertex_index = face_vertices_offset + (u + v * cube_face_resolution);
      const glm::vec3 offset = cube_face.u * (static_cast<float>(u) / static_cast<float>(cube_face_resolution - 1)) +
                               cube_face.v * (static_cast<float>(v) / static_cast<float>(cube_face_resolution - 1));
      engine::Vertex vertex{};
      vertex.position = CubeToSphere(cube_face.origin + offset);
      // TODO Normal
      vertex.uv = SphereToUV(vertex.position);
      vertices[vertex_index] = vertex;

      if (u < cube_face_resolution - 1 && v < cube_face_resolution - 1) {
        indices[face_indices_offset + indices_index++] = vertex_index;
        indices[face_indices_offset + indices_index++] = vertex_index + cube_face_resolution;
        indices[face_indices_offset + indices_index++] = vertex_index + cube_face_resolution + 1;
        indices[face_indices_offset + indices_index++] = vertex_index;
        indices[face_indices_offset + indices_index++] = vertex_index + cube_face_resolution + 1;
        indices[face_indices_offset + indices_index++] = vertex_index + 1;
      }
    }
  }
}

}  // namespace

namespace engine {
Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
  CreateVertexBuffer(device, vertices);
  CreateIndexBuffer(device, indices);
}

Mesh::~Mesh() = default;

std::unique_ptr<Mesh> Mesh::CreateSphereMesh(Device& device, uint32_t cube_face_resolution) {
  // Minimum corner XYZ -1 and maximum corner XYZ +1
  constexpr CubeFace back{
      .index = 0,
      .origin = glm::vec3{-1.0f, -1.0f, -1.0f},
      .u = 2.0f * glm::vec3{1.0f, 0.0f, 0.0f},
      .v = 2.0f * glm::vec3{0.0f, 1.0f, 0.0f},
      .normal = glm::vec3{0.0f, 0.0f, -1.0f},
  };
  constexpr CubeFace right{
      .index = 1,
      .origin = glm::vec3{1.0f, -1.0f, -1.0f},
      .u = 2.0f * glm::vec3{0.0f, 0.0f, 1.0f},
      .v = 2.0f * glm::vec3{0.0f, 1.0f, 0.0f},
      .normal = glm::vec3{1.0f, 0.0f, 0.0f},
  };
  constexpr CubeFace bottom{
      .index = 2,
      .origin = glm::vec3{-1.0f, -1.0f, -1.0f},
      .u = 2.0f * glm::vec3{0.0f, 0.0f, 1.0f},
      .v = 2.0f * glm::vec3{1.0f, 0.0f, 0.0f},
      .normal = glm::vec3{0.0f, -1.0f, 0.0f},
  };
  constexpr CubeFace front{
      .index = 3,
      .origin = glm::vec3{-1.0f, -1.0f, 1.0f},
      .u = 2.0f * glm::vec3{0.0f, 1.0f, 0.0f},
      .v = 2.0f * glm::vec3{1.0f, 0.0f, 0.0f},
      .normal = glm::vec3{0.0f, 0.0f, 1.0f},
  };
  constexpr CubeFace left{
      .index = 4,
      .origin = glm::vec3{-1.0f, -1.0f, -1.0f},
      .u = 2.0f * glm::vec3{0.0f, 1.0f, 0.0f},
      .v = 2.0f * glm::vec3{0.0f, 0.0f, 1.0f},
      .normal = glm::vec3{-1.0f, 0.0f, 0.0f},
  };
  constexpr CubeFace top{
      .index = 5,
      .origin = glm::vec3{-1.0f, 1.0f, -1.0f},
      .u = 2.0f * glm::vec3{1.0f, 0.0f, 0.0f},
      .v = 2.0f * glm::vec3{0.0f, 0.0f, 1.0f},
      .normal = glm::vec3{0.0f, 1.0f, 0.0f},
  };

  const uint32_t cube_face_vertex_count = cube_face_resolution * cube_face_resolution;
  const uint32_t cube_face_index_count = (cube_face_resolution - 1) * (cube_face_resolution - 1) * 6;
  std::vector<Vertex> vertices(cube_face_vertex_count * 6);
  std::vector<uint32_t> indices(cube_face_index_count * 6);

  GenerateCubeFace(back, cube_face_resolution, vertices, indices);
  GenerateCubeFace(right, cube_face_resolution, vertices, indices);
  GenerateCubeFace(bottom, cube_face_resolution, vertices, indices);
  GenerateCubeFace(front, cube_face_resolution, vertices, indices);
  GenerateCubeFace(left, cube_face_resolution, vertices, indices);
  GenerateCubeFace(top, cube_face_resolution, vertices, indices);

  return std::make_unique<Mesh>(device, vertices, indices);
}

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
