#pragma once

#include "engine/math.h"

namespace engine {
struct GlobalUniformBufferObject {
  glm::mat4 projection;
  glm::mat4 view;

  glm::vec4 ambient_light_color{1.0f, 1.0f, 1.0f, 0.05f};  // w is intensity
  glm::vec3 light_position{1.0f};
  alignas(16) glm::vec4 light_color{2.0f};  // w is intensity
};
}  // namespace engine
