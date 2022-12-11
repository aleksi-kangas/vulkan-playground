#include "engine/systems/model_render_system.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace {
struct PushConstants {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
};
}  // namespace

namespace engine::systems {
ModelRenderSystem::ModelRenderSystem(Device& device, VkRenderPass render_pass) : device_{device} {
  CreatePipelineLayout();
  CreatePipeline(render_pass);
}

ModelRenderSystem::~ModelRenderSystem() {
  vkDestroyPipelineLayout(device_.GetHandle(), pipeline_layout_, nullptr);
}

void ModelRenderSystem::Render(VkCommandBuffer command_buffer, const std::vector<std::unique_ptr<Model>>& models) {
  assert(pipeline_);
  pipeline_->Bind(command_buffer);

  for (const auto& model : models) {
    PushConstants push_constants{
        .model = model->GetTransform().Mat4(),
        .view = glm::lookAt(glm::vec3(-100.0f, -100.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f),
    };
    vkCmdPushConstants(command_buffer, pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants),
                       &push_constants);

    model->Bind(command_buffer);
    model->Draw(command_buffer);
  }
}

void ModelRenderSystem::CreatePipelineLayout() {
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(PushConstants);

  VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.setLayoutCount = 0;
  pipeline_layout_create_info.pSetLayouts = nullptr;
  pipeline_layout_create_info.pushConstantRangeCount = 1;
  pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;

  if (vkCreatePipelineLayout(device_.GetHandle(), &pipeline_layout_create_info, nullptr, &pipeline_layout_) !=
      VK_SUCCESS) {
    throw std::runtime_error{"Failed to create pipeline layout!"};
  }
}

void ModelRenderSystem::CreatePipeline(VkRenderPass render_pass) {
  assert(pipeline_layout_);

  GraphicsPipelineConfig pipeline_config = GraphicsPipelineConfig::Default();
  pipeline_config.pipeline_layout = pipeline_layout_;
  pipeline_config.render_pass = render_pass;
  pipeline_ =
      std::make_unique<GraphicsPipeline>(device_, pipeline_config, "shaders/model.vert.spv", "shaders/model.frag.spv");
}

}  // namespace engine::systems
