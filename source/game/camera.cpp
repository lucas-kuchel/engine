#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/settings.hpp>

#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/queue.hpp>

#include <array>
#include <cstring>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void createCamera(Camera& camera, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<glm::mat4, 2> matrices = {
            camera.projection,
            camera.view,
        };

        renderer::BufferCreateInfo bufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = matrices.size() * sizeof(glm::mat4),
        };

        camera.uniformBuffer = renderer::Buffer::create(bufferCreateInfo);

        std::size_t uniformBufferSize = renderer::Buffer::size(camera.uniformBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, uniformBufferSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), matrices.data(), uniformBufferSize);

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion bufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = uniformBufferSize,
        };

        stagingBufferOffset += uniformBufferSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, camera.uniformBuffer, {bufferCopyRegion});
    }

    void updateCamera(Camera& camera, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        // --- Aspect ratio and half-size like before ---
        float minExtent = static_cast<float>(std::min(camera.extent.x, camera.extent.y));
        float halfSize = camera.scale * 0.5f;
        float halfHeight = halfSize * (static_cast<float>(camera.extent.y) / minExtent);
        float aspect = static_cast<float>(camera.extent.x) / static_cast<float>(camera.extent.y);

        // --- Perspective parameters ---
        const float FOV_DEGREES = 45.0f;    // vertical FOV
        const float PITCH_DEGREES = -35.0f; // negative = look downward
        const float NEAR_MIN = 0.05f;
        const float FAR_EXTRA = 1000.0f;

        float fovRad = glm::radians(FOV_DEGREES);
        float pitchRad = glm::radians(PITCH_DEGREES);

        // Compute distance so vertical half-height matches ortho half-height (adjust for pitch)
        float tanHalfFov = std::tan(fovRad * 0.5f);
        float cameraDistance = (halfHeight / tanHalfFov) / std::cos(pitchRad);

        // --- Build rotation matrices ---
        // NOTE: negate yaw here to match the original ortho rotation sign/handedness.
        glm::mat4 rotYaw = glm::rotate(glm::mat4(1.0f), -camera.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotPitch = glm::rotate(glm::mat4(1.0f), pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));

        // Apply yaw first, then pitch (rotation = rotYaw * rotPitch)
        glm::mat4 rotation = rotYaw * rotPitch;

        // --- Compute camera forward vector (local -Z -> world)
        glm::vec3 forward = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));

        // --- Compute camera position so screen center hits camera.position.xy at Z=0 ---
        glm::vec3 target = glm::vec3(-camera.position, 0.0f); // Z = 0 plane center
        glm::vec3 cameraPos = target - forward * cameraDistance;

        // --- Perspective projection (zero-to-one depth for Vulkan) ---
        camera.projection = glm::perspective(fovRad, aspect, std::max(NEAR_MIN, cameraDistance * 0.01f), cameraDistance + FAR_EXTRA);
        camera.projection[1][1] *= -1.0f; // flip Y for Vulkan

        // --- Build view matrix (up = +Z) ---
        glm::mat4 flipXY = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 1.0f));
        camera.view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 0.0f, 1.0f)) * flipXY;

        // --- Upload matrices into uniform buffer ---
        std::array<glm::mat4, 2> matrices = {camera.projection, camera.view};
        std::size_t uniformBufferSize = renderer::Buffer::size(camera.uniformBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, uniformBufferSize, stagingBufferOffset);
        std::memcpy(mapping.data.data(), matrices.data(), uniformBufferSize);
        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion bufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = uniformBufferSize,
        };
        stagingBufferOffset += uniformBufferSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, camera.uniformBuffer, {bufferCopyRegion});
    }

    void destroyCamera(Camera& camera) {
        if (camera.uniformBuffer) {
            renderer::Buffer::destroy(camera.uniformBuffer);
        }
    }

    void easeCameraTowards(Camera& camera, glm::vec2 position, float deltaTime) {
        float frameEase = 1.0f - std::pow(1.0f - camera.ease, deltaTime);
        glm::vec2 delta = position - camera.position;
        camera.position += delta * frameEase;
    }
}