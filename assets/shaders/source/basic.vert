#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec2 vertexPosition;

layout(location = 1) in vec3 instanceTranslation;
layout(location = 2) in vec2 instanceScale;
layout(location = 3) in vec2 instanceShear;

layout(location = 4) in float instanceRotation;

layout(location = 5) in vec2 instanceTextureLocation;
layout(location = 6) in vec2 instanceTextureOffset;
layout(location = 7) in vec2 instanceTextureScale;

layout(location = 0) out vec2 outLocalPosition;
layout(location = 1) out vec2 outTextureLocation;
layout(location = 2) out vec2 outTextureOffset;

void main() {
    vec2 position = vertexPosition * instanceScale;

    float rotation = radians(instanceRotation);
    float cosRotation = cos(rotation);
    float sinRotation = sin(rotation);

    position = vec2(position.x + instanceShear.x * position.y, position.y + instanceShear.y * position.x);
    position = vec2(position.x * cosRotation - position.y * sinRotation, position.x * sinRotation + position.y * cosRotation);

    vec3 worldPosition = vec3(position, 0.0) + instanceTranslation;

    gl_Position = camera.projection * camera.view * vec4(worldPosition, 1.0);

    vec2 texturePosition = vertexPosition + vec2(0.5, 0.5);
    vec2 texturePositionScale = vec2(abs(instanceScale.x), -abs(instanceScale.y));

    outLocalPosition = texturePosition * texturePositionScale * instanceTextureScale;
    outTextureLocation = instanceTextureLocation;
    outTextureOffset = instanceTextureOffset;
}
