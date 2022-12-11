#include "engine/graphics_pipeline.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>

#include "engine/utils.h"

namespace engine {
GraphicsPipelineConfig GraphicsPipelineConfig::Default() {
  GraphicsPipelineConfig config{};

  config.vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  config.vertex_input_info.vertexBindingDescriptionCount =
      static_cast<uint32_t>(config.vertex_binding_descriptions.size());
  config.vertex_input_info.pVertexBindingDescriptions = config.vertex_binding_descriptions.data();
  config.vertex_input_info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(config.vertex_attribute_descriptions.size());
  config.vertex_input_info.pVertexAttributeDescriptions = config.vertex_attribute_descriptions.data();

  config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

  config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  config.viewport_info.viewportCount = 1;
  config.viewport_info.pViewports = nullptr;
  config.viewport_info.scissorCount = 1;
  config.viewport_info.pScissors = nullptr;

  config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  config.rasterization_info.depthClampEnable = VK_FALSE;
  config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
  config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
  config.rasterization_info.lineWidth = 1.0f;
  config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
  config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  config.rasterization_info.depthBiasEnable = VK_FALSE;
  config.rasterization_info.depthBiasConstantFactor = 0.0f;
  config.rasterization_info.depthBiasClamp = 0.0f;
  config.rasterization_info.depthBiasSlopeFactor = 0.0f;

  config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  config.multisample_info.sampleShadingEnable = VK_FALSE;
  config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  config.multisample_info.minSampleShading = 1.0f;
  config.multisample_info.pSampleMask = nullptr;
  config.multisample_info.alphaToCoverageEnable = VK_FALSE;
  config.multisample_info.alphaToOneEnable = VK_FALSE;

  config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  config.depth_stencil_info.depthTestEnable = VK_TRUE;
  config.depth_stencil_info.depthWriteEnable = VK_TRUE;
  config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
  config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
  config.depth_stencil_info.minDepthBounds = 0.0f;
  config.depth_stencil_info.maxDepthBounds = 1.0f;
  config.depth_stencil_info.stencilTestEnable = VK_FALSE;
  config.depth_stencil_info.front = {};
  config.depth_stencil_info.back = {};

  config.color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  config.color_blend_attachment.blendEnable = VK_FALSE;
  config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  config.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  config.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  config.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  config.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  config.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  config.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  config.color_blend_info.logicOpEnable = VK_FALSE;
  config.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
  config.color_blend_info.attachmentCount = 1;
  config.color_blend_info.pAttachments = &config.color_blend_attachment;
  config.color_blend_info.blendConstants[0] = 0.0f;
  config.color_blend_info.blendConstants[1] = 0.0f;
  config.color_blend_info.blendConstants[2] = 0.0f;
  config.color_blend_info.blendConstants[3] = 0.0f;

  config.dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  config.dynamic_info.dynamicStateCount = static_cast<uint32_t>(config.dynamic_states.size());
  config.dynamic_info.pDynamicStates = config.dynamic_states.data();

  config.pipeline_layout = VK_NULL_HANDLE;
  config.render_pass = VK_NULL_HANDLE;
  config.subpass = 0;

  config.base_pipeline_handle = VK_NULL_HANDLE;
  config.base_pipeline_index = -1;

  return config;
}

GraphicsPipeline::GraphicsPipeline(Device& device, const GraphicsPipelineConfig& config,
                                   const std::filesystem::path& vertex_shader_path,
                                   const std::filesystem::path& fragment_shader_path)
    : device_{device} {
  CreateShaderModule(vertex_shader_path, &vertex_shader_module_);
  CreateShaderModule(fragment_shader_path, &fragment_shader_module_);
  CreateGraphicsPipeline(config);
}

GraphicsPipeline::~GraphicsPipeline() {
  vkDestroyShaderModule(device_.GetHandle(), vertex_shader_module_, nullptr);
  vkDestroyShaderModule(device_.GetHandle(), fragment_shader_module_, nullptr);
  vkDestroyPipeline(device_.GetHandle(), pipeline_, nullptr);
}

void GraphicsPipeline::Bind(VkCommandBuffer command_buffer) {
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}

void GraphicsPipeline::CreateShaderModule(const std::filesystem::path& shader_path, VkShaderModule* shader_module) {
  assert(shader_path.has_filename());
  assert(shader_path.has_extension());
  assert(shader_path.extension() == ".spv");
  const auto code = utils::ReadFile(shader_path);

  VkShaderModuleCreateInfo shader_module_info{};
  shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_module_info.codeSize = code.size();
  shader_module_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
  if (vkCreateShaderModule(device_.GetHandle(), &shader_module_info, nullptr, shader_module) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create shader module: " + shader_path.string()};
  }
}

void GraphicsPipeline::CreateGraphicsPipeline(const GraphicsPipelineConfig& config) {
  assert(config.pipeline_layout != VK_NULL_HANDLE);
  assert(config.render_pass != VK_NULL_HANDLE);
  assert(vertex_shader_module_ != VK_NULL_HANDLE);
  assert(fragment_shader_module_ != VK_NULL_HANDLE);

  std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{{
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .module = vertex_shader_module_,
          .pName = "main",
      },
      {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = fragment_shader_module_,
          .pName = "main",
      },
  }};

  VkGraphicsPipelineCreateInfo graphics_pipeline_info{};
  graphics_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  graphics_pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
  graphics_pipeline_info.pStages = shader_stages.data();
  graphics_pipeline_info.pVertexInputState = &config.vertex_input_info;
  graphics_pipeline_info.pInputAssemblyState = &config.input_assembly_info;
  graphics_pipeline_info.pViewportState = &config.viewport_info;
  graphics_pipeline_info.pRasterizationState = &config.rasterization_info;
  graphics_pipeline_info.pMultisampleState = &config.multisample_info;
  graphics_pipeline_info.pDepthStencilState = &config.depth_stencil_info;
  graphics_pipeline_info.pColorBlendState = &config.color_blend_info;
  graphics_pipeline_info.pDynamicState = &config.dynamic_info;
  graphics_pipeline_info.layout = config.pipeline_layout;
  graphics_pipeline_info.renderPass = config.render_pass;
  graphics_pipeline_info.subpass = config.subpass;
  graphics_pipeline_info.basePipelineHandle = config.base_pipeline_handle;
  graphics_pipeline_info.basePipelineIndex = config.base_pipeline_index;

  if (vkCreateGraphicsPipelines(device_.GetHandle(), VK_NULL_HANDLE, 1, &graphics_pipeline_info, nullptr, &pipeline_) !=
      VK_SUCCESS) {
    throw std::runtime_error{"Failed to create graphics pipeline!"};
  }
}

}  // namespace engine