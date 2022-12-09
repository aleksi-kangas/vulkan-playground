#include "engine/window.h"

#include <utility>

namespace engine {
Window::Window(std::string title, uint32_t width, uint32_t height)
    : title_{std::move(title)}, width_{static_cast<int32_t>(width)}, height_{static_cast<int32_t>(height)} {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // TODO For now...
  window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, FramebufferResizeCallback);
  glfwSetKeyCallback(window_, ExitOnEscapeCallback);
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

bool Window::ShouldClose() const {
  return glfwWindowShouldClose(window_);
}

void Window::CreateSurface(VkInstance instance, VkSurfaceKHR* surface) {
  if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create window surface!"};
  }
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int32_t width, int32_t height) {
  auto* window_ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  window_ptr->width_ = width;
  window_ptr->height_ = height;
}

void Window::ExitOnEscapeCallback(GLFWwindow* window, int32_t key, int32_t /* scancode */, int32_t action,
                                  int32_t /* mods */) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

}  // namespace engine
