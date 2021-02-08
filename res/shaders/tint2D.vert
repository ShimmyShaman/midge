#version 450
#extension GL_ARB_separate_shader_objects : enable
// #extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform UBO0 {
    mat4 mvp;
} globalUI;
layout (binding = 1) uniform UBO1 {
    vec2 offset;
    vec2 scale;
} element;

layout(location = 0) in vec2 inPosition;

void main() {
    gl_Position = globalUI.mvp * vec4(inPosition, 0.0, 1.0);
    gl_Position.xy *= element.scale.xy;
    gl_Position.xy += element.offset.xy;
}