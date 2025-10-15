#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/settings.hpp>

#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/queue.hpp>

#include <array>
#include <cstring>

#include <glm/gtc/constants.hpp>
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
        float extentX = static_cast<float>(std::max(camera.extent.x, 1u));
        float extentY = static_cast<float>(std::max(camera.extent.y, 1u));
        float aspect = extentX / extentY;

        // --- Vulkan-friendly perspective (RH, depth 0..1) ---
        float fovRad = glm::radians(camera.fov);

        // --- build orientation using yaw (Y axis) and pitch (X axis) ---
        // camera.rotation.x == pitch (degrees)  -> rotation about X (look up/down)
        // camera.rotation.y == yaw   (degrees)  -> rotation about Y (turn left/right)
        float pitchRad = glm::radians(camera.rotation.x);
        float yawRad = glm::radians(camera.rotation.y);

        // Build rotations: yaw then pitch (yaw rotates around world Y, pitch about local X)
        glm::mat4 rotYaw = glm::rotate(glm::mat4(1.0f), yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotPitch = glm::rotate(glm::mat4(1.0f), pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotation = rotYaw * rotPitch;

        // Local forward is -Z; rotate to world space
        glm::vec3 forward = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // halfHeight calculation - if you have a 'scale' field similar to the ortho camera:
        float minExtent = static_cast<float>(std::min(camera.extent.x, camera.extent.y));
        float halfHeight = camera.scale * (static_cast<float>(camera.extent.y) / minExtent);

        float tanHalfFov = std::tan(fovRad * 0.5f);
        // compensate for pitch so the projected vertical half-height matches `halfHeight`
        float cameraDistance = (halfHeight / tanHalfFov) / std::cos(pitchRad);

        // treat camera.position.xz as the world centre you want on-screen
        glm::vec3 groundCenter = glm::vec3(camera.position.x, 0.0f, camera.position.z);
        glm::vec3 cameraPosAbove = groundCenter - forward * cameraDistance;

        camera.projection = glm::perspectiveRH_ZO(fovRad, aspect, std::max(camera.near, cameraDistance * 0.01f), cameraDistance + 1000.0f);
        camera.projection[1][1] *= -1.0f;

        camera.view = glm::lookAt(cameraPosAbove, groundCenter, up);

        std::array<glm::mat4, 2> matrices = {
            camera.projection,
            camera.view,
        };

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

    void easeCameraTowards(Camera& camera, glm::vec3 position, float deltaTime) {
        float frameEase = 1.0f - std::pow(1.0f - camera.ease, deltaTime);
        glm::vec3 delta = position - camera.position;
        camera.position += delta * frameEase;
    }
}