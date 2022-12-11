#include "engine/application.h"

#include <chrono>

#include <vulkan/vulkan.h>

namespace engine {
Application::Application(const ApplicationInfo& application_info)
    : window_{application_info.title, application_info.window_width, application_info.window_height} {}

void Application::Run() {
  auto frame_start_time = std::chrono::high_resolution_clock::now();

  while (!window_.ShouldClose()) {
    glfwPollEvents();
    auto current_time = std::chrono::high_resolution_clock::now();
    const float frame_time =
        std::chrono::duration<float, std::chrono::seconds::period>(current_time - frame_start_time).count();
    frame_start_time = current_time;

    OnFrame(frame_time);
    DrawFrame();
  }
  vkDeviceWaitIdle(device_.GetHandle());
}

void Application::DrawFrame() {
  if (auto command_buffer = renderer_.BeginFrame()) {
    renderer_.BeginRenderPass(command_buffer);

    model_render_system_.Render(command_buffer, models_);

    renderer_.EndRenderPass(command_buffer);
    renderer_.EndFrame();
  }
}

}  // namespace engine
