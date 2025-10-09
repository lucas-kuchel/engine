#version 450

layout(set = 0, binding = 1) uniform sampler2D inTex0;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColour;

void main() {
    outColour = texture(inTex0, inTexCoord);
}