#pragma once

#include <filesystem>
#include <vector>

namespace engine::utils {
std::vector<char> ReadFile(const std::filesystem::path& file_path);
}  // namespace engine::utils