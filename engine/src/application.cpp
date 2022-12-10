#include "engine/application.h"

#include <vulkan/vulkan.h>

namespace engine {
Application::Application(const ApplicationInfo& application_info)
    : window_{application_info.title, application_info.window_width, application_info.window_height} {}

void Application::Run() {
  while (!window_.ShouldClose()) {
    glfwPollEvents();
  }
  vkDeviceWaitIdle(device_.GetHandle());
}

}  // namespace engine
