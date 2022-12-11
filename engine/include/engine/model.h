#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <utility>

#include "engine/device.h"
#include "engine/mesh.h"
#include "engine/vertex.h"

namespace engine {
struct ModelLoader {
  std::shared_ptr<Mesh> mesh;

  void Load(Device& device, const std::filesystem::path& file_path);
};

class Model {
 public:
  Model() = default;
  ~Model() = default;

  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

  static std::unique_ptr<Model> CreateFromFile(Device& device, const std::filesystem::path& file_path);

  void AttachMesh(std::shared_ptr<Mesh> mesh) { mesh_ = std::move(mesh); }

  void Bind(VkCommandBuffer command_buffer) const;
  void Draw(VkCommandBuffer command_buffer) const;

 private:
  std::shared_ptr<Mesh> mesh_;
};
}  // namespace engine
