#pragma once

#include <cstdint>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace engine {
enum class Keys {
  kEscape = GLFW_KEY_ESCAPE,
  kW = GLFW_KEY_W,
  kA = GLFW_KEY_A,
  kS = GLFW_KEY_S,
  kD = GLFW_KEY_D,
  kQ = GLFW_KEY_Q,
  kE = GLFW_KEY_E,
  kLeft = GLFW_KEY_LEFT,
  kRight = GLFW_KEY_RIGHT,
  kUp = GLFW_KEY_UP,
  kDown = GLFW_KEY_DOWN,
};

class Window {
 public:
  Window(std::string title, uint32_t width, uint32_t height);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  [[nodiscard]] VkExtent2D GetExtent() const { return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)}; }
  void GetCursorPosition(double& x, double& y) const { return glfwGetCursorPos(window_, &x, &y); }
  [[nodiscard]] bool HasResized() const { return framebuffer_resized_; }
  [[nodiscard]] bool IsKeyPressed(Keys key) const {
    return glfwGetKey(window_, static_cast<int32_t>(key)) == GLFW_PRESS;
  }
  void ResetResizedFlag() { framebuffer_resized_ = false; }
  [[nodiscard]] bool ShouldClose() const;

  void CreateSurface(VkInstance instance, VkSurfaceKHR* surface);

 private:
  GLFWwindow* window_;
  std::string title_;
  int32_t width_;
  int32_t height_;
  bool framebuffer_resized_ = false;

  static void FramebufferResizeCallback(GLFWwindow* glfw_window, int32_t width, int32_t height);
  static void ExitOnEscapeCallback(GLFWwindow* glfw_window, int32_t key, int32_t scancode, int32_t action,
                                   int32_t mods);
};
}  // namespace engine