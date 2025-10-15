#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec3 vertexPosition;

layout(location = 1) in vec2 instanceTextureLocation;
layout(location = 2) in vec2 instanceTextureOffset;
layout(location = 3) in vec2 instanceTextureScale;
layout(location = 4) in mat4 instanceModel;

layout(location = 0) out vec2 outLocalPosition;
layout(location = 1) out vec2 outTextureLocation;
layout(location = 2) out vec2 outTextureOffset;

void main() {
    gl_Position = camera.projection * camera.view * vec4(vertexPosition, 1.0);

    vec3 instanceScale = {1.0, 1.0, 1.0};

    instanceScale.x = length(instanceModel[0].xyz);
    instanceScale.y = length(instanceModel[1].xyz);
    instanceScale.z = length(instanceModel[2].xyz);

    vec2 texturePosition = vertexPosition.xz + vec2(0.5, 0.5);
    vec2 texturePositionScale = vec2(abs(instanceScale.x), -abs(instanceScale.y));

    outLocalPosition = texturePosition * texturePositionScale * instanceTextureScale;
    outTextureLocation = instanceTextureLocation;
    outTextureOffset = instanceTextureOffset;
}
