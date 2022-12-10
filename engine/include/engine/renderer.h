#pragma once

#include <memory>

#include "engine/device.h"
#include "engine/swap_chain.h"
#include "engine/window.h"

namespace engine {
class Renderer {
 public:
  Renderer(Window& window, Device& device);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

 private:
  Window& window_;
  Device& device_;

  std::unique_ptr<Swapchain> swap_chain_;
  uint32_t image_index_ = 0;

  VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;

  void AllocateCommandBuffers();
  void FreeCommandBuffers();

};
}  // namespace engine
