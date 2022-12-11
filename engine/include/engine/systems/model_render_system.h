#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "engine/device.h"
#include "engine/graphics_pipeline.h"
#include "engine/model.h"

namespace engine::systems {
class ModelRenderSystem {
 public:
  ModelRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_descriptor_set_layout);
  ~ModelRenderSystem();

  ModelRenderSystem(const ModelRenderSystem&) = delete;
  ModelRenderSystem& operator=(const ModelRenderSystem&) = delete;

  void Render(VkCommandBuffer command_buffer, const std::vector<std::unique_ptr<Model>>& models,
              VkDescriptorSet global_descriptor_set);

 private:
  Device& device_;

  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  std::unique_ptr<GraphicsPipeline> pipeline_;

  void CreatePipelineLayout(VkDescriptorSetLayout global_descriptor_set_layout);
  void CreatePipeline(VkRenderPass render_pass);
};
}  // namespace engine::systems
