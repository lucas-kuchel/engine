#version 450

layout(set = 0, binding = 1) uniform sampler2D inAlbedoTexture;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) in vec2 inTextureLocation;
layout(location = 2) in vec2 inTextureOffset;

layout(location = 0) out vec4 outColour;

void main() {
    vec2 fractionalLocalPosition = fract(inLocalPosition + inTextureOffset);
    vec2 scaledLocalPosition = fractionalLocalPosition * vec2(0.5, 0.5);
    vec2 texturePosition = scaledLocalPosition + inTextureLocation;

    outColour = texture(inAlbedoTexture, texturePosition);

    if (outColour.a == 0.0) {
        discard;
    }
}