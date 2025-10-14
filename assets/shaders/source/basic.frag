#version 450

layout(set = 0, binding = 1) uniform sampler2D inAlbedoTexture;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) in vec2 inTextureOffset;

layout(location = 0) out vec4 outColour;

void main() {
    vec2 fractionalLocalPosition = fract(inLocalPosition);
    vec2 scaledLocalPosition = fractionalLocalPosition * vec2(0.5, 0.5);
    vec2 texturePosition = scaledLocalPosition + inTextureOffset;

    outColour = texture(inAlbedoTexture, texturePosition);
    // outColour = vec4(texturePosition, 0.0, 1.0);
}