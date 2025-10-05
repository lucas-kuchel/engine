#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outColour;
layout(location = 1) out vec2 outTexCoord;

void main() {
    vec4 position = vec4(inPosition, 0.0, 1.0);

    gl_Position = position;
    outColour = inColour;
    outTexCoord = inTexCoord;
}
