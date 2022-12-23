#version 450

layout (location = 0) out vec2 fragOffset;

layout (set = 0, binding = 0) uniform UniformBufferObject {
  mat4 projection;
  mat4 view;

  vec4 ambientLightColor;  // w is intensity
  vec3 lightPosition;
  vec4 lightColor;  // w is intensity
} ubo;

// Hardcode billboard vertex offsets
const vec2 OFFSETS[6] = vec2[](
vec2(-1.0, -1.0),
vec2(-1.0, 1.0),
vec2(1.0, -1.0),
vec2(1.0, -1.0),
vec2(-1.0, 1.0),
vec2(1.0, 1.0)
);

const float LIGHT_RADIUS = 0.1f;

void main() {
  vec3 cameraRightWorld = { ubo.view[0][0], ubo.view[1][0], ubo.view[2][0] };
  vec3 cameraUpWorld = { ubo.view[0][1], ubo.view[1][1], ubo.view[2][1] };

  vec3 positionWorld = ubo.lightPosition.xyz + cameraRightWorld * OFFSETS[gl_VertexIndex].x * LIGHT_RADIUS + cameraUpWorld * OFFSETS[gl_VertexIndex].y * LIGHT_RADIUS;
  gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);

  fragOffset = OFFSETS[gl_VertexIndex];
}