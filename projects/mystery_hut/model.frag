#version 450
#extension GL_ARB_separate_shader_objects : enable
// #extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D texSampler;

layout (location = 0) out vec4 outColor;

layout(location = 1) in vec2 fragTexCoord;

void main() {
  // outColor = vec4(0.4, 0.2, 0.8, 1.0);
  outColor = texture(texSampler, fragTexCoord);
}