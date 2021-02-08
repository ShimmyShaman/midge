#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 2) uniform UBO2 {
    vec4 tint;
    vec4 texCoordBounds;
} element;

layout(binding = 3) uniform sampler2D texSampler;

layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 texCoords = vec2(
        element.texCoordBounds.x + fragTexCoord.x * (element.texCoordBounds.y - element.texCoordBounds.x),
        element.texCoordBounds.z + fragTexCoord.y * (element.texCoordBounds.w - element.texCoordBounds.z));
    outColor = texture(texSampler, texCoords);
    if(outColor.r < 0.01)
        discard;
    // outColor.a = 0.3 + 0.7 * outColor.r;
    outColor.a = min(max(0, outColor.r - 0.2) * 0.2f + outColor.r * 1.5, 1.0);
    outColor.rgb = element.tint.rgb * outColor.a;
    outColor.a = 1.0f;
}