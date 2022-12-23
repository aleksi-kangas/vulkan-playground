#include "engine/transform.h"

namespace engine {
glm::mat4 Transform::Mat4() const {
  auto transform = glm::mat4{1.0f};
  transform = glm::translate(transform, translation);
  transform = glm::scale(transform, glm::vec3{scale, scale, scale});
  return transform;
}

}  // namespace engine
