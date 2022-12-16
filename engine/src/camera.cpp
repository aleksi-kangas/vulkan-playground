#include "engine/camera.h"

#include <limits>

namespace engine {

void Camera::ProcessInput(float delta_time) {
  bool view_changed = false;

  // Rotation
  glm::vec3 rotate{0.0f};
  if (window_.IsKeyPressed(Keys::kLeft))
    rotate.y -= 1.0f;
  if (window_.IsKeyPressed(Keys::kRight))
    rotate.y += 1.0f;
  if (window_.IsKeyPressed(Keys::kUp))
    rotate.x += 1.0f;
  if (window_.IsKeyPressed(Keys::kDown))
    rotate.x -= 1.0f;
  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
    rotation_ += glm::normalize(rotate) * delta_time * kLookSpeed;
    rotation_.x = glm::clamp(rotation_.x, -kMaxPitch, kMaxPitch);
    rotation_.y = glm::mod(rotation_.y, glm::two_pi<float>());
    view_changed = true;
  }

  const float yaw = rotation_.y;
  const glm::vec3 forward_direction{sin(yaw), 0.0f, cos(yaw)};
  const glm::vec3 right_direction{forward_direction.z, 0.0f, -forward_direction.x};
  const glm::vec3 up_direction{0.0f, -1.0f, 0.0f};

  // Translation
  glm::vec3 translation{};
  if (window_.IsKeyPressed(Keys::kW))
    translation += forward_direction;
  if (window_.IsKeyPressed(Keys::kA))
    translation -= right_direction;
  if (window_.IsKeyPressed(Keys::kS))
    translation -= forward_direction;
  if (window_.IsKeyPressed(Keys::kD))
    translation += right_direction;
  if (window_.IsKeyPressed(Keys::kE))
    translation += up_direction;
  if (window_.IsKeyPressed(Keys::kQ))
    translation -= up_direction;
  if (glm::dot(translation, translation) > std::numeric_limits<float>::epsilon()) {
    position_ += glm::normalize(translation) * delta_time * kMoveSpeed;
    view_changed = true;
  }

  if (view_changed) {
    RecomputeView();
  }
}

void Camera::SetPerspective(float fov_y, float aspect, float near, float far) {
  // Need to explicitly use left-handed perspective, because we use a coordinate system where:
  // +X is right, -Y is up, and +Z is forward.
  projection_ = glm::perspectiveLH(fov_y, aspect, near, far);
}

void Camera::RecomputeView() {
  view_ = glm::inverse(glm::eulerAngleYXZ(rotation_.y, rotation_.x, rotation_.z)) * glm::translate(glm::mat4{1.0f}, -position_);
}

}  // namespace engine
