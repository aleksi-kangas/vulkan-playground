#version 450

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec3 fragColor;
layout (location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform UniformBufferObject {
  mat4 projection;
  mat4 view;

  vec4 ambientLightColor;  // w is intensity
  vec3 lightPosition;
  vec4 lightColor;  // w is intensity
} ubo;

layout (set = 1, binding = 0) uniform sampler2D texSampler;

void main() {
  vec3 directionToLight = ubo.lightPosition - fragPosition;
  vec3 attenuation = vec3(1.0f) / dot(directionToLight, directionToLight);

  vec3 ambientLightColor = ubo.ambientLightColor.rgb * ubo.ambientLightColor.w;
  vec3 lightColor = ubo.lightColor.rgb * ubo.lightColor.w * attenuation;
  vec3 diffuseLight = lightColor * max(dot(normalize(fragNormal), normalize(directionToLight)), 0.0f);

  vec3 color = (diffuseLight + ambientLightColor) * texture(texSampler, fragUV).rgb;
//  vec3 fragColor = (diffuseLight + ambientLightColor) * color;

  outColor = vec4(color, 1.0f);
}
