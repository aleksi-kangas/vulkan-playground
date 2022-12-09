#include "engine/window.h"

int main() {
  engine::Window window{"Vulkan", 800, 600};
  while (!window.ShouldClose()) {
    glfwPollEvents();
  }
}
