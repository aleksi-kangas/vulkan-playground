#include "engine/camera.h"

#include <limits>

namespace engine {

void Camera::ProcessInput(float delta_time) {
  bool view_changed = false;

  const glm::vec3 forward_direction = glm::normalize(glm::vec3{
      cos(rotation_.y) * cos(rotation_.x),
      sin(rotation_.x),
      sin(rotation_.y) * cos(rotation_.x),
  });
  const glm::vec3 right_direction = glm::normalize(glm::cross(forward_direction, kWorldUp));
  const glm::vec3 up_direction = glm::normalize(glm::cross(right_direction, forward_direction));

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
  projection_ = glm::perspective(fov_y, aspect, near, far);
  projection_[1][1] *= -1.0f;
}

void Camera::RecomputeView() {
  const float pitch = rotation_.x;
  const float yaw = rotation_.y;
  const glm::vec3 forward_direction = glm::normalize(glm::vec3{
      cos(yaw) * cos(pitch),
      sin(pitch),
      sin(yaw) * cos(pitch),
  });

  view_ = glm::lookAt(position_, position_ + forward_direction, kWorldUp);
}

}  // namespace engine
