#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec3 vertexPosition;

layout(location = 1) in vec2 instanceTexturePosition;
layout(location = 2) in vec2 instanceTextureExtent;
layout(location = 3) in vec2 instanceTextureOffset;
layout(location = 4) in vec2 instanceTextureScale;

layout(location = 5) in mat4 instanceModel;

layout(location = 0) out vec2 outLocalPosition;
layout(location = 1) out vec2 outTexturePosition;
layout(location = 2) out vec2 outTextureExtent;
layout(location = 3) out vec2 outTextureOffset;

void main() {
    gl_Position = camera.projection * camera.view * instanceModel * vec4(vertexPosition, 1.0);

    vec2 texturePosition = vec2(vertexPosition.x, -vertexPosition.y) + vec2(0.5, 0.5);

    outLocalPosition = texturePosition * instanceTextureScale + instanceTextureOffset;
    outTexturePosition = instanceTexturePosition;
    outTextureExtent = instanceTextureExtent;
    outTextureOffset = instanceTextureOffset;
}
