#include "engine/application.h"

#include <chrono>

#include <vulkan/vulkan.h>

namespace {
VkPipelineLayout CreatePipelineLayout(engine::Device& device) {
  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  if (vkCreatePipelineLayout(device.GetHandle(), &pipeline_layout_create_info, nullptr, &pipeline_layout) !=
      VK_SUCCESS) {
    throw std::runtime_error{"Failed to create pipeline layout!"};
  }

  return pipeline_layout;
}
}  // namespace

namespace engine {
Application::Application(const ApplicationInfo& application_info)
    : window_{application_info.title, application_info.window_width, application_info.window_height} {}

void Application::Run() {
  auto frame_start_time = std::chrono::high_resolution_clock::now();

  // TODO Move to a proper place...
  engine::GraphicsPipelineConfig graphics_pipeline_config = engine::GraphicsPipelineConfig::Default();
  graphics_pipeline_config.pipeline_layout = CreatePipelineLayout(device_);
  graphics_pipeline_config.render_pass = renderer_.GetRenderPass();
  engine::GraphicsPipeline pipeline{device_, graphics_pipeline_config, "shaders/model.vert.spv",
                                    "shaders/model.frag.spv"};

  while (!window_.ShouldClose()) {
    glfwPollEvents();
    auto current_time = std::chrono::high_resolution_clock::now();
    const float frame_time =
        std::chrono::duration<float, std::chrono::seconds::period>(current_time - frame_start_time).count();
    frame_start_time = current_time;

    OnFrame(frame_time);
    DrawFrame(pipeline);
  }
  vkDeviceWaitIdle(device_.GetHandle());

  // TODO Move to a proper place...
  vkDestroyPipelineLayout(device_.GetHandle(), graphics_pipeline_config.pipeline_layout, nullptr);
}

void Application::DrawFrame(engine::GraphicsPipeline& pipeline) {
  if (auto command_buffer = renderer_.BeginFrame()) {
    renderer_.BeginRenderPass(command_buffer);

    pipeline.Bind(command_buffer);

    model_->Bind(command_buffer);
    model_->Draw(command_buffer);

    renderer_.EndRenderPass(command_buffer);
    renderer_.EndFrame();
  }
}

}  // namespace engine
