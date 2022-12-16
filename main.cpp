#include <cstdlib>
#include <iostream>

#include "engine/application.h"
#include "engine/mesh.h"
#include "engine/vertex.h"

class HelloTriangleApplication : public engine::Application {
 public:
  explicit HelloTriangleApplication(const engine::ApplicationInfo& application_info = {.title = "Hello Triangle"})
      : engine::Application{application_info} {
    models_.emplace_back(engine::Model::CreateFromFile(device_, "assets/viking_room.obj"));
    models_.back()->AttachTexture(engine::Texture::CreateFromFile(texture_manager_, "assets/viking_room.png"));
  }

  void OnFrame(float frame_time) override {}
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
