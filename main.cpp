#include <cstdlib>
#include <iostream>

#include "engine/application.h"

class HelloTriangleApplication : public engine::Application {
 public:
  explicit HelloTriangleApplication(const engine::ApplicationInfo& application_info = {.title = "Hello Triangle"})
      : engine::Application{application_info} {}

  void OnFrame(float dt) override {}
};

int main() {
  try {
    HelloTriangleApplication application;
    application.Run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
