#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec2 vertexPosition;

layout(location = 1) in vec4 instancePosition;
layout(location = 2) in vec2 instanceScale;

layout(location = 3) in vec2 instanceTexturePosition;
layout(location = 4) in vec2 instanceTextureExtent;
layout(location = 5) in vec2 instanceTextureOffset;
layout(location = 6) in vec2 instanceTextureRepeat;
layout(location = 7) in float instanceOpacity;

layout(location = 0) out vec2 outLocalPosition;
layout(location = 1) out vec2 outTexturePosition;
layout(location = 2) out vec2 outTextureExtent;
layout(location = 3) out float outOpacity;

void main() {
    vec3 scaledPosition = vec3(vertexPosition * instanceScale, 0.0);
    vec4 translatedPosition = vec4(scaledPosition, 0.0) + instancePosition;

    gl_Position = camera.projection * camera.view * translatedPosition;
    outLocalPosition = vertexPosition * instanceTextureRepeat + instanceTextureOffset;
    outTexturePosition = instanceTexturePosition;
    outTextureExtent = instanceTextureExtent;
    outOpacity = instanceOpacity;
}
