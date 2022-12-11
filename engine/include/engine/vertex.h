#pragma once

#include <array>

#include <vulkan/vulkan.h>

#include "engine/math.h"
#include "engine/utils.h"

namespace engine {
struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec3 color{};
  glm::vec2 uv{};

  bool operator==(const Vertex& other) const {
    return position == other.position && normal == other.normal && color == other.color && uv == other.uv;
  }

  static std::array<VkVertexInputBindingDescription, 1> BindingDescriptions();
  static std::array<VkVertexInputAttributeDescription, 4> AttributeDescriptions();
};
}  // namespace engine

namespace std {
template <>
struct hash<engine::Vertex> {
  size_t operator()(engine::Vertex const& vertex) const {
    size_t seed = 0;
    engine::utils::HashCombine(seed, vertex.position, vertex.color, vertex.uv);
    return seed;
  }
};
}  // namespace std
