#pragma once

#include <array>
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

  [[nodiscard]] float GetAspectRatio() const { return swap_chain_->GetAspectRatio(); }
  [[nodiscard]] uint32_t GetFrameIndex() const { return frame_index_; }
  [[nodiscard]] VkRenderPass GetRenderPass() const { return swap_chain_->GetRenderPass(); }

  VkCommandBuffer BeginFrame();
  void BeginRenderPass(VkCommandBuffer command_buffer) const;

  void EndRenderPass(VkCommandBuffer command_buffer) const;
  void EndFrame();

 private:
  Window& window_;
  Device& device_;

  std::unique_ptr<Swapchain> swap_chain_;
  uint32_t image_index_ = 0;

  uint32_t frame_index_ = 0;  // [0, Swapchain::kMaxFramesInFlight)

  std::array<VkCommandBuffer, Swapchain::kMaxFramesInFlight> command_buffers_{};

  [[nodiscard]] VkCommandBuffer GetCurrentCommandBuffer() const { return command_buffers_[frame_index_]; }

  void AllocateCommandBuffers();
  void FreeCommandBuffers();

  void RecreateSwapchain();
};
}  // namespace engine
