#version 450

layout(set = 0, binding = 0) uniform sampler2D tex0;

layout(location = 0) in vec3 inColour;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColour;

void main() {
    outColour = texture(tex0, inTexCoord);
}
