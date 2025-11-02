#version 450

layout(set = 0, binding = 1) uniform sampler2D inAlbedoTexture;
layout(set = 0, binding = 2) uniform sampler2D inNormalTexture;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) in vec2 inTexturePosition;
layout(location = 2) in vec2 inTextureExtent;
layout(location = 3) in vec4 inColourFactor;

layout(location = 0) out vec4 outColour;

const vec3 lightDirection = normalize(vec3(0.2, 0.5, 0.3));

void main() {
    vec2 fractionalLocalPosition = fract(inLocalPosition);
    vec2 scaledLocalPosition = fractionalLocalPosition * inTextureExtent;
    vec2 texturePosition = scaledLocalPosition + inTexturePosition;

    vec4 albedo = texture(inAlbedoTexture, texturePosition);
    vec3 normal = texture(inNormalTexture, texturePosition).xyz;
    float diffuse = max(dot(normal, lightDirection), 0.0);

    outColour = diffuse * (albedo * inColourFactor);

    if (outColour.a == 0.0) {
        discard;
    }
}