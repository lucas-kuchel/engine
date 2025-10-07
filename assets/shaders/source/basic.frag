#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColour;

void main() {
    outColour = vec4(inTexCoord, 0.0, 1.0);
}
