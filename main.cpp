#include "engine/device.h"
#include "engine/window.h"

int main() {
  engine::Window window{"Vulkan Playground", 800, 600};
  engine::Device device{window};
  while (!window.ShouldClose()) {
    glfwPollEvents();
  }
}
