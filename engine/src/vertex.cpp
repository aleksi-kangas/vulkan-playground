#include "engine/vertex.h"

namespace engine {
std::array<VkVertexInputBindingDescription, 1> Vertex::BindingDescriptions() {
  return {{{
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  }}};
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::AttributeDescriptions() {
  return {{{
               .location = 0,
               .binding = 0,
               .format = VK_FORMAT_R32G32B32_SFLOAT,
               .offset = offsetof(Vertex, position),
           },
           {
               .location = 1,
               .binding = 0,
               .format = VK_FORMAT_R32G32B32_SFLOAT,
               .offset = offsetof(Vertex, normal),
           },
           {
               .location = 2,
               .binding = 0,
               .format = VK_FORMAT_R32G32B32_SFLOAT,
               .offset = offsetof(Vertex, color),
           },
           {
               .location = 3,
               .binding = 0,
               .format = VK_FORMAT_R32G32_SFLOAT,
               .offset = offsetof(Vertex, uv),
           }}};
}

}  // namespace engine
