#pragma once

#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include "engine/device.h"
#include "engine/mesh.h"
#include "engine/transform.h"
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

  Transform& GetTransform() { return transform_; }
  void AttachMesh(std::shared_ptr<Mesh> mesh) { mesh_ = std::move(mesh); }

  void Bind(VkCommandBuffer command_buffer) const;
  void Draw(VkCommandBuffer command_buffer) const;

 private:
  Transform transform_;

  std::shared_ptr<Mesh> mesh_;
};
}  // namespace engine
