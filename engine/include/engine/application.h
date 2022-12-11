#pragma once

#include <cstdint>
#include <string>

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
  virtual ~Application() = default;

  virtual void OnFrame(float frame_time) = 0;

  Window window_;
  Device device_{window_};
  Renderer renderer_{window_, device_};

  std::vector<std::unique_ptr<Model>> models_;

 private:
  engine::systems::ModelRenderSystem model_render_system_{device_, renderer_.GetRenderPass()};

  void DrawFrame();
};
}  // namespace engine
