#include "engine/systems/model_render_system.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace {
struct PushConstants {
  glm::mat4 model;
};
}  // namespace

namespace engine::systems {
ModelRenderSystem::ModelRenderSystem(Device& device, VkRenderPass render_pass,
                                     VkDescriptorSetLayout global_descriptor_set_layout)
    : device_{device} {
  CreatePipelineLayout(global_descriptor_set_layout);
  CreatePipeline(render_pass);
}

ModelRenderSystem::~ModelRenderSystem() {
  vkDestroyPipelineLayout(device_.GetHandle(), pipeline_layout_, nullptr);
}

void ModelRenderSystem::Render(VkCommandBuffer command_buffer, const std::vector<std::unique_ptr<Model>>& models,
                               VkDescriptorSet global_descriptor_set) {
  assert(pipeline_);
  pipeline_->Bind(command_buffer);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
                          &global_descriptor_set, 0, nullptr);

  for (const auto& model : models) {
    PushConstants push_constants{
        .model = model->GetTransform().Mat4(),
    };
    vkCmdPushConstants(command_buffer, pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants),
                       &push_constants);

    model->Bind(command_buffer);
    model->Draw(command_buffer);
  }
}

void ModelRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout global_descriptor_set_layout) {
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(PushConstants);

  std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts = {global_descriptor_set_layout};

  VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
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
