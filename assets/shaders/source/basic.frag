#version 450

layout(set = 0, binding = 0) uniform materialBuffer {
    vec3 colour;
}
material;

layout(location = 0) out vec4 outColour;

void main() {
    outColour = vec4(material.colour, 1.0);
}
