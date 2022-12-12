#include "engine/utils.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace engine::utils {
std::vector<char> ReadFile(const std::filesystem::path& file_path) {
  std::ifstream file{file_path, std::ios::ate | std::ios::binary};
  if (!file.is_open()) {
    throw std::runtime_error{"Failed to open file: " + file_path.string()};
  }
  const size_t file_size = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(file_size));
  file.close();
  return buffer;
}

std::vector<uint8_t> ReadImage(const std::filesystem::path& image_path, uint32_t& width, uint32_t& height,
                               uint32_t& channels) {
  assert(image_path.has_filename());
  assert(image_path.has_extension());

  int32_t w, h, c;
  stbi_uc* pixels = stbi_load(image_path.c_str(), &w, &h, &c, STBI_rgb_alpha);
  if (!pixels) {
    throw std::runtime_error{"Failed to load texture image!"};
  }
  std::vector<uint8_t> buffer(w * h * 4);
  std::memcpy(buffer.data(), pixels, buffer.size());
  width = static_cast<uint32_t>(w);
  height = static_cast<uint32_t>(h);
  channels = static_cast<uint32_t>(c);
  return buffer;
}

}  // namespace engine::utils