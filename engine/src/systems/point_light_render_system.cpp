#include "engine/systems/point_light_render_system.h"

#include <array>

namespace engine::systems {
PointLightRenderSystem::PointLightRenderSystem(Device& device, VkRenderPass render_pass,
                                               VkDescriptorSetLayout global_descriptor_set_layout)
    : device_{device} {
  CreatePipelineLayout(global_descriptor_set_layout);
  CreatePipeline(render_pass);
}

PointLightRenderSystem::~PointLightRenderSystem() {
  vkDestroyPipelineLayout(device_.GetHandle(), pipeline_layout_, nullptr);
}

void PointLightRenderSystem::Render(VkCommandBuffer command_buffer, VkDescriptorSet global_descriptor_set) {
  assert(pipeline_);
  pipeline_->Bind(command_buffer);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
                          &global_descriptor_set, 0, nullptr);

  vkCmdDraw(command_buffer, 6, 1, 0, 0);
}

void PointLightRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout global_descriptor_set_layout) {
  std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts{global_descriptor_set_layout};

  VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
  pipeline_layout_create_info.pushConstantRangeCount = 0;
  pipeline_layout_create_info.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(device_.GetHandle(), &pipeline_layout_create_info, nullptr, &pipeline_layout_) !=
      VK_SUCCESS) {
    throw std::runtime_error{"Failed to create pipeline layout!"};
  }
}

void PointLightRenderSystem::CreatePipeline(VkRenderPass render_pass) {
  assert(pipeline_layout_);

  GraphicsPipelineConfig pipeline_config = GraphicsPipelineConfig::Default();
  pipeline_config.vertex_input_info.vertexAttributeDescriptionCount = 0;
  pipeline_config.vertex_input_info.pVertexAttributeDescriptions = nullptr;
  pipeline_config.vertex_input_info.vertexBindingDescriptionCount = 0;
  pipeline_config.vertex_input_info.pVertexBindingDescriptions = nullptr;
  pipeline_config.pipeline_layout = pipeline_layout_;
  pipeline_config.render_pass = render_pass;
  pipeline_ = std::make_unique<GraphicsPipeline>(device_, pipeline_config, "shaders/point_light.vert.spv",
                                                 "shaders/point_light.frag.spv");
}

}  // namespace engine::systems
