#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec2 vertexPosition;

layout(location = 1) in vec2 instancePosition;
layout(location = 2) in vec2 instanceScale;

layout(location = 3) in vec2 instanceTexturePosition;
layout(location = 4) in vec2 instanceTextureExtent;
layout(location = 5) in vec2 instanceTextureOffset;
layout(location = 6) in vec2 instanceTextureRepeat;
layout(location = 7) in vec4 instanceColourFactor;

layout(location = 0) out vec2 outLocalPosition;
layout(location = 1) out vec2 outTexturePosition;
layout(location = 2) out vec2 outTextureExtent;
layout(location = 3) out vec4 outColourFactor;

void main() {
    vec4 transformedPosition = vec4(vertexPosition + instancePosition * instanceScale, -0.5, 1.0);
    vec2 textureSamplePosition = vec2(vertexPosition.x, -vertexPosition.y);

    gl_Position = camera.projection * camera.view * transformedPosition;
    outLocalPosition = textureSamplePosition * instanceTextureRepeat + instanceTextureOffset;
    outTexturePosition = instanceTexturePosition;
    outTextureExtent = instanceTextureExtent;
    outColourFactor = instanceColourFactor;
}
