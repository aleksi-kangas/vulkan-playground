#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "engine/camera.h"
#include "engine/device.h"
#include "engine/model.h"
#include "engine/renderer.h"
#include "engine/systems/model_render_system.h"
#include "engine/window.h"

namespace engine {
struct ApplicationInfo {
  std::string title = "Application";
  uint32_t window_width = 800;
  uint32_t window_height = 600;
};

class Application {
 public:
  void Run();

 protected:
  explicit Application(const ApplicationInfo& application_info);
  virtual ~Application();

  virtual void OnFrame(float frame_time) = 0;

  Window window_;
  Device device_{window_};
  Renderer renderer_{window_, device_};
  Camera camera_{};

  std::vector<std::unique_ptr<Model>> models_;

 private:
  std::unique_ptr<engine::systems::ModelRenderSystem> model_render_system_;

  // TODO Abstraction?
  std::vector<std::unique_ptr<Buffer>> uniform_buffers_{Swapchain::kMaxFramesInFlight};
  VkDescriptorPool global_descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout global_descriptor_set_layout_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> global_descriptor_sets_{Swapchain::kMaxFramesInFlight, VK_NULL_HANDLE};

  void DrawFrame();
};
}  // namespace engine
