#pragma once

#include "engine/math.h"

namespace engine {
class Camera {
 public:
  Camera() = default;

  Camera(const Camera&) = default;
  Camera& operator=(const Camera&) = default;

  Camera(Camera&&) = default;
  Camera& operator=(Camera&&) = default;

  [[nodiscard]] const glm::mat4& GetProjection() const { return projection_; }
  [[nodiscard]] const glm::mat4& GetView() const { return view_; }

  void SetPerspective(float fov, float aspect, float near, float far);
  void SetViewDirection(const glm::vec3& position, const glm::vec3& view_direction, glm::vec3 up = {0.0f, -1.0f, 0.0f});
  void SetViewTarget(const glm::vec3& position, const glm::vec3& target, glm::vec3 up = {0.0f, -1.0f, 0.0f});

 private:
  glm::mat4 projection_{1.0f};
  glm::mat4 view_{1.0f};
};
}  // namespace engine
