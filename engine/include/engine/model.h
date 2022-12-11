#pragma once

#include <filesystem>
#include <memory>
#include <utility>

#include "engine/device.h"
#include "engine/mesh.h"

namespace engine {
class Model {
 public:
  Model() = default;
  ~Model() = default;

  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;

  void AttachMesh(std::shared_ptr<Mesh> mesh) { mesh_ = std::move(mesh); }

  void Bind(VkCommandBuffer command_buffer) const;
  void Draw(VkCommandBuffer command_buffer) const;

 private:
  std::shared_ptr<Mesh> mesh_;
};
}  // namespace engine
