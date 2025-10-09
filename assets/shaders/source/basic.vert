#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

layout(location = 2) in vec3 instanceTranslation;
layout(location = 3) in vec2 instanceScale;
layout(location = 4) in vec2 instanceTexOffset;

layout(location = 0) out vec2 outTexCoord;

void main() {
    vec2 scaled = vertexPosition * instanceScale;
    vec3 offset = vec3(scaled, 0.0) + instanceTranslation;
    vec4 transformed = camera.projection * camera.view * vec4(offset, 1.0);

    gl_Position = transformed;
    outTexCoord = vertexTexCoord + instanceTexOffset;
}
