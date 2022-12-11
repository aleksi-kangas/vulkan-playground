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
  ModelRenderSystem(Device& device, VkRenderPass render_pass);
  ~ModelRenderSystem();

  ModelRenderSystem(const ModelRenderSystem&) = delete;
  ModelRenderSystem& operator=(const ModelRenderSystem&) = delete;

  void Render(VkCommandBuffer command_buffer, const std::vector<std::unique_ptr<Model>>& models);

 private:
  Device& device_;

  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  std::unique_ptr<GraphicsPipeline> pipeline_;

  void CreatePipelineLayout();
  void CreatePipeline(VkRenderPass render_pass);
};
}  // namespace engine::systems
