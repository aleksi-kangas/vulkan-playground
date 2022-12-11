#pragma once

#include "engine/math.h"

namespace engine {
struct Transform {
  glm::vec3 translation{0.0f, 0.0f, 0.0f};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  // TODO Rotation

  [[nodiscard]] glm::mat4 Mat4() const;
};
}  // namespace engine
