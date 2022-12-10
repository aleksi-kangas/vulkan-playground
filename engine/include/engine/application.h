#pragma once

#include <cstdint>
#include <string>

#include "engine/device.h"
#include "engine/renderer.h"
#include "engine/window.h"

namespace engine {
struct ApplicationInfo {
  std::string title = "Application";
  uint32_t window_width = 800;
  uint32_t window_height = 600;
};

class Application {
 public:
  explicit Application(const ApplicationInfo& application_info);
  virtual ~Application() = default;

  virtual void OnFrame(float frame_time) = 0;

  void Run();

 private:
  Window window_;
  Device device_{window_};
  Renderer renderer_{window_, device_};

  void DrawFrame();
};
}  // namespace engine
