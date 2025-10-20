#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
}
camera;

layout(location = 0) in vec2 vertexPosition;

layout(location = 1) in vec2 instanceTexturePosition;
layout(location = 2) in vec2 instanceTextureExtent;
layout(location = 3) in vec2 instanceTextureOffset;
layout(location = 4) in vec2 instanceTextureScale;

layout(location = 5) in vec4 instancePosition;
layout(location = 6) in mat2 instanceModel;

layout(location = 0) out vec2 outLocalPosition;
layout(location = 1) out vec2 outTexturePosition;
layout(location = 2) out vec2 outTextureExtent;

void main() {
    // calculate position of the vertex
    vec3 transformedPosition = vec3(instanceModel * vertexPosition, 0.0);
    vec4 translatedPosition = instancePosition + vec4(transformedPosition, 0.0);

    // put vertex into world space
    gl_Position = camera.projection * camera.view * translatedPosition;

    // output the local texture position to the fragment shader
    outLocalPosition = vertexPosition * instanceTextureScale + instanceTextureOffset;

    // output atlas-local transforms to the fragment shader
    outTexturePosition = instanceTexturePosition;
    outTextureExtent = instanceTextureExtent;
}
