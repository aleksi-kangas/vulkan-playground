#include "engine/application.h"

#include <chrono>

#include <vulkan/vulkan.h>

#include "engine/uniforms.h"

namespace engine {
Application::Application(const ApplicationInfo& application_info)
    : window_{application_info.title, application_info.window_width, application_info.window_height} {

  for (uint32_t i = 0; i < Swapchain::kMaxFramesInFlight; ++i) {
    uniform_buffers_[i] =
        std::make_unique<Buffer>(device_, sizeof(GlobalUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniform_buffers_[i]->Map();
  }

  // Descriptor set layout
  std::array<VkDescriptorSetLayoutBinding, 1> global_descriptor_set_layout_bindings{
      {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}},
  };
  VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
  descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(global_descriptor_set_layout_bindings.size());
  descriptor_set_layout_info.pBindings = global_descriptor_set_layout_bindings.data();
  if (vkCreateDescriptorSetLayout(device_.GetHandle(), &descriptor_set_layout_info, nullptr,
                                  &global_descriptor_set_layout_) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create descriptor set layout"};
  }

  // Descriptor sets
  std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts{global_descriptor_set_layout_};
  for (uint32_t i = 0; i < Swapchain::kMaxFramesInFlight; ++i) {
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool = device_.GetDescriptorPool();
    descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    descriptor_set_allocate_info.pSetLayouts = descriptor_set_layouts.data();
    if (vkAllocateDescriptorSets(device_.GetHandle(), &descriptor_set_allocate_info, &global_descriptor_sets_[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error{"Failed to allocate descriptor sets"};
    }

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = uniform_buffers_[i]->GetHandle();
    buffer_info.offset = 0;
    buffer_info.range = sizeof(GlobalUniformBufferObject);

    std::array<VkWriteDescriptorSet, 1> descriptor_writes{
        {{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, global_descriptor_sets_[i], 0, 0, 1,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &buffer_info, nullptr}},
    };
    vkUpdateDescriptorSets(device_.GetHandle(), static_cast<uint32_t>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);
  }

  model_render_system_ =
      std::make_unique<systems::ModelRenderSystem>(device_, renderer_.GetRenderPass(), global_descriptor_set_layout_);
  point_light_render_system_ = std::make_unique<systems::PointLightRenderSystem>(device_, renderer_.GetRenderPass(),
                                                                                 global_descriptor_set_layout_);
}

Application::~Application() {
  vkDestroyDescriptorSetLayout(device_.GetHandle(), global_descriptor_set_layout_, nullptr);
}

void Application::Run() {
  auto frame_start_time = std::chrono::high_resolution_clock::now();

  while (!window_.ShouldClose()) {
    glfwPollEvents();
    auto current_time = std::chrono::high_resolution_clock::now();
    const float frame_time =
        std::chrono::duration<float, std::chrono::seconds::period>(current_time - frame_start_time).count();
    frame_start_time = current_time;

    camera_.ProcessInput(frame_time);
    OnFrame(frame_time);
    DrawFrame();
  }
  vkDeviceWaitIdle(device_.GetHandle());
}

void Application::DrawFrame() {
  camera_.SetPerspective(glm::radians(45.0f), renderer_.GetAspectRatio(), 0.1f, 1000.0f);

  if (auto command_buffer = renderer_.BeginFrame()) {
    // Update
    GlobalUniformBufferObject ubo{
        .projection = camera_.GetProjection(),
        .view = camera_.GetView(),
    };
    uniform_buffers_[renderer_.GetFrameIndex()]->Write(&ubo);
    uniform_buffers_[renderer_.GetFrameIndex()]->Flush();

    // Render
    renderer_.BeginRenderPass(command_buffer);

    model_render_system_->Render(command_buffer, models_, global_descriptor_sets_[renderer_.GetFrameIndex()]);
    point_light_render_system_->Render(command_buffer, global_descriptor_sets_[renderer_.GetFrameIndex()]);

    renderer_.EndRenderPass(command_buffer);
    renderer_.EndFrame();
  }
}

}  // namespace engine
