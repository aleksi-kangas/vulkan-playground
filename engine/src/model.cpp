#include "engine/model.h"

namespace engine {
void Model::Bind(VkCommandBuffer command_buffer) const {
  assert(mesh_);
  mesh_->Bind(command_buffer);
}

void Model::Draw(VkCommandBuffer command_buffer) const {
  assert(mesh_);
  mesh_->Draw(command_buffer);
}

}  // namespace engine