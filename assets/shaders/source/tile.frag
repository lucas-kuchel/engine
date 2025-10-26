#version 450

layout(set = 0, binding = 1) uniform sampler2D inAlbedoTexture;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) in vec2 inTexturePosition;
layout(location = 2) in vec2 inTextureExtent;
layout(location = 3) in vec3 inColourFactor;

layout(location = 0) out vec4 outColour;

void main() {
    vec2 fractionalLocalPosition = fract(inLocalPosition);
    vec2 scaledLocalPosition = fractionalLocalPosition * inTextureExtent;
    vec2 texturePosition = scaledLocalPosition + inTexturePosition;

    outColour = texture(inAlbedoTexture, texturePosition);
    outColour.xyz *= inColourFactor;

    if (outColour.a == 0.0) {
        discard;
    }
}