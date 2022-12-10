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

  [[nodiscard]] VkExtent2D GetExtent() const { return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)}; }
  [[nodiscard]] bool HasResized() const { return framebuffer_resized_; }
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
  static void ExitOnEscapeCallback(GLFWwindow* glfw_window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
};
}  // namespace engine