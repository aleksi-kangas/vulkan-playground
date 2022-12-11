#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform UniformBufferObject {
  mat4 projection;
  mat4 view;
} ubo;

layout (push_constant) uniform PushConstant {
  mat4 model;
} pushConstant;

void main() {
  gl_Position = ubo.projection * ubo.view * pushConstant.model * vec4(position, 1.0);
  fragColor = color;
}
