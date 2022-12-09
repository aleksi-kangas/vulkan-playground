#pragma once

#include <cstdint>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace engine {
class Window {
 public:
  Window(std::string title, uint32_t width, uint32_t height);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  [[nodiscard]] bool ShouldClose() const;

 private:
  GLFWwindow* window_;
  std::string title_;
  int32_t width_;
  int32_t height_;

  static void FramebufferResizeCallback(GLFWwindow* window, int32_t width, int32_t height);
  static void ExitOnEscapeCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
};
}  // namespace engine