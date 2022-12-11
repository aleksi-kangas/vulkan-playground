#pragma once

#include <cstdint>
#include <string>

#include "engine/device.h"
#include "engine/model.h"
#include "engine/renderer.h"
#include "engine/window.h"

// TODO Move to a proper place...
#include "engine/graphics_pipeline.h"

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

  std::unique_ptr<engine::Model> model_;

 private:
  void DrawFrame(engine::GraphicsPipeline& graphics_pipeline);
};
}  // namespace engine
