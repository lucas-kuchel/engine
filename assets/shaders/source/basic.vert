#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
}
pushConstants;

layout(set = 1, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;

void main() {
    vec4 position = camera.projection * camera.view * pushConstants.model * vec4(inPosition, 0.0, 1.0);

    gl_Position = position;
    outTexCoord = inTexCoord;
    outPosition = position.xyz;
}
