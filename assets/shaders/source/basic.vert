#version 450

layout(push_constant) uniform PushConstants {
    vec2 offset;
}
pushConstants;

layout(location = 0) in vec2 inPosition;

void main() {
    vec4 position = vec4(inPosition + pushConstants.offset, 0.0, 1.0);

    gl_Position = position;
}
