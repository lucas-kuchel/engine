#version 450

layout(set = 0, binding = 1) uniform sampler2D inAlbedoTexture;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) in vec2 inTexturePosition;
layout(location = 2) in vec2 inTextureExtent;

layout(location = 0) out vec4 outColour;

void main() {
    vec2 fractionalLocalPosition = fract(inLocalPosition);
    vec2 scaledLocalPosition = fractionalLocalPosition * inTextureExtent;
    vec2 texturePosition = clamp(
        scaledLocalPosition + inTexturePosition,
        inTexturePosition,
        inTexturePosition + inTextureExtent - 1e-6);

    outColour = texture(inAlbedoTexture, texturePosition);

    if (outColour.a == 0.0) {
        discard;
    }
}