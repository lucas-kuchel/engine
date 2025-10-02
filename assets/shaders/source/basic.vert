#version 450

layout(push_constant) uniform PushConstants {
    vec2 offset;
}
pushConstants;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColour;

layout(location = 0) out vec3 outColour;

void main() {
    gl_Position = vec4(inPosition + pushConstants.offset, 0.0, 1.0);

    outColour = inColour;
}
