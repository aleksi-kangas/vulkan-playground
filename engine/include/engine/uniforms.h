#pragma once

#include "engine/math.h"

namespace engine {
struct GlobalUniformBufferObject {
  glm::mat4 projection;
  glm::mat4 view;
};
}  // namespace engine
