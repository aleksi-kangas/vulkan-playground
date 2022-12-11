#pragma once

#include <array>

#include <vulkan/vulkan.h>

#include "engine/math.h"

namespace engine {
struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec3 color{};
  glm::vec2 uv{};

  static std::array<VkVertexInputBindingDescription, 1> BindingDescriptions();
  static std::array<VkVertexInputAttributeDescription, 4> AttributeDescriptions();
};
}  // namespace engine
