#pragma once

#include <vulkan/vulkan.h>

#include "engine/device.h"
#include "engine/graphics_pipeline.h"

namespace engine::systems {
class PointLightRenderSystem {
 public:
  PointLightRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_descriptor_set_layout);
  ~PointLightRenderSystem();

  PointLightRenderSystem(const PointLightRenderSystem&) = delete;
  PointLightRenderSystem& operator=(const PointLightRenderSystem&) = delete;

  void Render(VkCommandBuffer command_buffer, VkDescriptorSet global_descriptor_set);

 private:
  Device& device_;

  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  std::unique_ptr<GraphicsPipeline> pipeline_;

  void CreatePipelineLayout(VkDescriptorSetLayout global_descriptor_set_layout);
  void CreatePipeline(VkRenderPass render_pass);
};
}  // namespace engine::systems
