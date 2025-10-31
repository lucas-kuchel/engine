#version 450

layout(set = 0, binding = 1) uniform sampler2D inAlbedoTexture;
layout(set = 0, binding = 2) uniform sampler2D inNormalTexture;
layout(set = 0, binding = 3) uniform sampler2D inSpecularTexture;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) in vec2 inTexturePosition;
layout(location = 2) in vec2 inTextureExtent;
layout(location = 3) in vec3 inColourFactor;

layout(location = 0) out vec4 outColour;

vec3 lightDir = normalize(vec3(1.0, 0.5, 0.25));

void main() {
    vec2 fractionalLocalPosition = fract(inLocalPosition * 0.125);
    vec2 scaledLocalPosition = fractionalLocalPosition * inTextureExtent;
    vec2 texturePosition = scaledLocalPosition + inTexturePosition;

    vec4 albedo = texture(inAlbedoTexture, texturePosition);
    vec3 normal = texture(inNormalTexture, texturePosition).rgb;
    float specular = texture(inSpecularTexture, texturePosition).r;

    vec3 V = normalize(vec3(0.0, 0.0, -1.0)); // camera looks along Z+
    vec3 L = normalize(lightDir);
    vec3 H = normalize(L + V);

    float specAngle = max(dot(normalize(normal), H), 0.0);
    float shininess = 16.0;              // hardcoded
    vec3 specularColor = vec3(specular); // use specular texture for intensity

    vec3 specularComponent = specularColor * pow(specAngle, shininess);

    vec3 diffuseComponent = (albedo.xyz * inColourFactor) * max(dot(normalize(normal), L), 0.0);

    vec3 finalColor = diffuseComponent + specularComponent;

    outColour = vec4(finalColor, albedo.a);

    if (outColour.a == 0.0) {
        discard;
    }
}