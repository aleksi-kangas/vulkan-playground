#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>

namespace engine::utils {
// Source: https://stackoverflow.com/a/2595226
template <typename T, typename... Rest>
void HashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (HashCombine(seed, rest), ...);
}

std::vector<char> ReadFile(const std::filesystem::path& file_path);

std::vector<uint8_t> ReadImage(const std::filesystem::path& image_path, uint32_t& width, uint32_t& height,
                               uint32_t& channels);
}  // namespace engine::utils