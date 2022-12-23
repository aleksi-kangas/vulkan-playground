#pragma once

#include <utility>

#include "engine/math.h"
#include "engine/window.h"

namespace engine {
class Camera {
 public:
  explicit Camera(Window& window) : window_{window} { RecomputeView(); }

  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  [[nodiscard]] const glm::mat4& GetProjection() const { return projection_; }
  [[nodiscard]] const glm::mat4& GetView() const { return view_; }

  void ProcessInput(float delta_time);
  void SetPerspective(float fov_y, float aspect, float near, float far);

 private:
  Window& window_;

  static constexpr float kMaxPitch = 1.55334;  // 89 degrees;
  static constexpr float kMoveSpeed = 4.0f;
  static constexpr float kLookSpeed = 1.5f;
  static constexpr glm::vec3 kWorldUp{0.0f, 1.0f, 0.0f};

  glm::vec3 position_{0.0f, 0.0f, 3.0f};
  glm::vec3 rotation_{0.0f, -glm::radians(90.0f), 0.0f};  // Pitch, Yaw, Roll

  glm::mat4 projection_{1.0f};
  glm::mat4 view_{1.0f};

  void RecomputeView();
};
}  // namespace engine
