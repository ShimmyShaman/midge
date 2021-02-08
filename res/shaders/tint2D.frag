#version 450
#extension GL_ARB_separate_shader_objects : enable
// #extension GL_ARB_shading_language_420pack : enable
layout (binding = 2) uniform UBO2 {
    vec4 tint;
} element;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = element.tint;
}