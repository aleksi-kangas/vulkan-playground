#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec3 fragColor;
layout (location = 3) out vec2 fragUV;

layout (set = 0, binding = 0) uniform UniformBufferObject {
  mat4 projection;
  mat4 view;

  vec4 ambientLightColor;  // w is intensity
  vec3 lightPosition;
  vec4 lightColor;  // w is intensity
} ubo;

layout (push_constant) uniform PushConstant {
  mat4 model;
} pushConstant;

void main() {
  vec4 positionWorld = pushConstant.model * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;

  fragPosition = positionWorld.xyz;
  fragNormal = normalize(mat3(pushConstant.model) * normal);
  fragColor = color;
  fragUV = uv;
}
