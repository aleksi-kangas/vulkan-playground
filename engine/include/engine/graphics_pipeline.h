#pragma once

#include <array>
#include <filesystem>
#include <vector>

#include <vulkan/vulkan.h>

#include "engine/device.h"
#include "engine/vertex.h"

namespace engine {
struct GraphicsPipelineConfig {
  VkPipelineCreateFlags flags = 0;

  std::array<VkVertexInputBindingDescription, 1> vertex_binding_descriptions = Vertex::BindingDescriptions();
  std::array<VkVertexInputAttributeDescription, 2> vertex_attribute_descriptions = Vertex::AttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
  VkPipelineViewportStateCreateInfo viewport_info{};
  VkPipelineRasterizationStateCreateInfo rasterization_info{};
  VkPipelineMultisampleStateCreateInfo multisample_info{};
  VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  VkPipelineColorBlendStateCreateInfo color_blend_info{};
  std::array<VkDynamicState, 2> dynamic_states{{
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  }};
  VkPipelineDynamicStateCreateInfo dynamic_info{};

  VkPipelineLayout pipeline_layout{};
  VkRenderPass render_pass{};
  uint32_t subpass{};
  VkPipeline base_pipeline_handle{};
  int32_t base_pipeline_index{};

  static GraphicsPipelineConfig Default();
};

class GraphicsPipeline {
 public:
  GraphicsPipeline(Device& device, const GraphicsPipelineConfig& config,
                   const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path);
  ~GraphicsPipeline();

  GraphicsPipeline(const GraphicsPipeline&) = delete;
  GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

  void Bind(VkCommandBuffer command_buffer);

 private:
  Device& device_;

  VkShaderModule vertex_shader_module_ = VK_NULL_HANDLE;
  VkShaderModule fragment_shader_module_ = VK_NULL_HANDLE;

  VkPipeline pipeline_ = VK_NULL_HANDLE;

  void CreateShaderModule(const std::filesystem::path& shader_path, VkShaderModule* shader_module);
  void CreateGraphicsPipeline(const GraphicsPipelineConfig& config);
};
}  // namespace engine