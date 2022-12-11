#include <cstdlib>
#include <iostream>

#include "engine/application.h"
#include "engine/mesh.h"
#include "engine/vertex.h"

class HelloTriangleApplication : public engine::Application {
 public:
  explicit HelloTriangleApplication(const engine::ApplicationInfo& application_info = {.title = "Hello Triangle"})
      : engine::Application{application_info} {
    model_ = std::make_unique<engine::Model>();

    const std::vector<engine::Vertex> kVertices{
        {
            {
                .position = {0.0f, -0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 0.0f},
                .color = {1.0f, 0.0f, 0.0f},
                .uv = {0.0f, 0.0f},
            },
            {
                .position = {0.5f, 0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 0.0f},
                .color = {0.0f, 1.0f, 0.0f},
                .uv = {0.0f, 0.0f},
            },
            {
                .position = {-0.5f, 0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 0.0f},
                .color = {0.0f, 0.0f, 1.0f},
                .uv = {0.0f, 0.0f},
            },
        },
    };

    auto triangle_mesh = std::make_shared<engine::Mesh>(device_, kVertices);
    model_->AttachMesh(triangle_mesh);
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
