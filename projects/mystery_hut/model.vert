#version 450
#extension GL_ARB_separate_shader_objects : enable
//  #extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform UBO0 {
  mat4 mvp;
} world;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 1) out vec2 fragTexCoord;

void main() {
  gl_Position = world.mvp * vec4(inPosition, 1.0);
  fragTexCoord = inTexCoord;
};