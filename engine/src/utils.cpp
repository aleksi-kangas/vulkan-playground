#include "engine/utils.h"

#include <fstream>
#include <stdexcept>

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

}  // namespace engine::utils