#include "engine/camera.h"

namespace engine {
void Camera::SetPerspective(float fov, float aspect, float near, float far) {
  projection_ = glm::perspective(fov, aspect, near, far);
}

void Camera::SetViewDirection(const glm::vec3& position, const glm::vec3& view_direction, glm::vec3 up) {
  view_ = glm::lookAt(position, position + view_direction, up);
}

void Camera::SetViewTarget(const glm::vec3& position, const glm::vec3& target, glm::vec3 up) {
  view_ = glm::lookAt(position, target, up);
}

}  // namespace engine
