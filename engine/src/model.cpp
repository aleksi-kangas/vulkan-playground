#include "engine/model.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace engine {
void ModelLoader::Load(Device& device, const std::filesystem::path& file_path) {
  assert(file_path.has_filename());
  assert(file_path.has_extension());
  assert(file_path.extension() == ".obj");

  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = file_path.parent_path().string();  // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(file_path.string(), reader_config)) {
    if (!reader.Error().empty()) {
      throw std::runtime_error{reader.Error()};
    }
  }

  if (!reader.Warning().empty()) {
    std::cerr << "TinyObjReader: " << reader.Warning();
  }

  const auto& attrib = reader.GetAttrib();
  const auto& shapes = reader.GetShapes();
  const auto& materials = reader.GetMaterials();

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::unordered_map<Vertex, uint32_t> unique_vertices{};
  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex vertex{};
      if (index.vertex_index >= 0) {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
        };
        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
      }
      if (index.normal_index >= 0) {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
        };
      }
      if (index.texcoord_index >= 0) {
        vertex.uv = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      if (!unique_vertices.contains(vertex)) {
        unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(unique_vertices[vertex]);
    }
  }

  mesh = std::make_unique<Mesh>(device, vertices, indices);
}

std::unique_ptr<Model> Model::CreateFromFile(Device& device, const std::filesystem::path& file_path) {
  ModelLoader model_loader{};
  model_loader.Load(device, file_path);
  auto model = std::make_unique<Model>();
  model->AttachMesh(std::move(model_loader.mesh));
  return model;
}

void Model::Bind(VkCommandBuffer command_buffer) const {
  assert(mesh_);
  mesh_->Bind(command_buffer);
}

void Model::Draw(VkCommandBuffer command_buffer) const {
  assert(mesh_);
  mesh_->Draw(command_buffer);
}

}  // namespace engine