#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/settings.hpp>
#include <game/transforms.hpp>

#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/queue.hpp>

#include <array>
#include <cstring>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void updateCameras(entt::registry& registry) {
        auto view = registry.view<Camera, Position, Rotation>();

        for (auto& entity : view) {
            auto& camera = registry.get<Camera>(entity);
            auto& target = registry.get<Position>(entity);
            auto& rotation = registry.get<Rotation>(entity);

            float extentX = static_cast<float>(std::max(camera.extent.x, 1u));
            float extentY = static_cast<float>(std::max(camera.extent.y, 1u));
            float aspect = extentX / extentY;

            float fovRadians = glm::radians(camera.fov);
            float pitchRadians = glm::radians(rotation.rotation.x);
            float yawRadians = glm::radians(rotation.rotation.y);

            glm::mat4 rotYaw = glm::rotate(glm::mat4{1.0f}, yawRadians, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotPitch = glm::rotate(glm::mat4{1.0f}, pitchRadians, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 rotationMatrix = rotYaw * rotPitch;

            glm::vec3 forward = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 position = target.position - forward * camera.scale;

            camera.projection = glm::perspectiveRH_ZO(
                fovRadians,
                aspect,
                camera.near,
                camera.far);
            camera.projection[1][1] *= -1.0f;

            camera.view = glm::lookAt(position, target.position, up);
        }
    }

    void createCameraBuffer(renderer::Device& device, renderer::Buffer& cameraBuffer, std::size_t size) {
        if (cameraBuffer) {
            return;
        }

        renderer::BufferCreateInfo bufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = size,
        };

        cameraBuffer = renderer::Buffer::create(bufferCreateInfo);
    }

    void updateCameraBuffer(renderer::Buffer& cameraBuffer, std::span<std::uint8_t> data, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::size_t stagingBufferOffset) {
        if (!cameraBuffer || !stagingBuffer) {
            return;
        }

        auto mapping = renderer::Buffer::map(stagingBuffer, data.size(), stagingBufferOffset);

        std::memcpy(mapping.data.data(), data.data(), data.size());

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion copyRegion = {
            .sizeBytes = data.size(),
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
        };

        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, cameraBuffer, {copyRegion});
    }
mera, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
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
    void createCamera(Camera& ca
            .sizeBytes = uniformBufferSize,
        };

        stagingBufferOffset += uniformBufferSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, camera.uniformBuffer, {bufferCopyRegion});
}

void updateCamera(Camera& camera, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
    float extentX = static_cast<float>(std::max(camera.extent.x, 1u));
    float extentY = static_cast<float>(std::max(camera.extent.y, 1u));
    float aspect = extentX / extentY;

    float fovRadians = glm::radians(camera.fov);
    float pitchRadians = glm::radians(camera.rotation.x);
    float yawRadians = glm::radians(camera.rotation.y);

    glm::mat4 rotYaw = glm::rotate(glm::mat4{1.0f}, yawRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotPitch = glm::rotate(glm::mat4{1.0f}, pitchRadians, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotation = rotYaw * rotPitch;

    glm::vec3 forward = glm::normalize(glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float distance = camera.scale;
    camera.position = camera.target - forward * distance;

    camera.projection = glm::perspectiveRH_ZO(
        fovRadians,
        aspect,
        camera.near,
        camera.far);
    camera.projection[1][1] *= -1.0f;

    camera.view = glm::lookAt(camera.position, camera.target, up);

    std::array<glm::mat4, 2> matrices = {
        camera.projection,
        camera.view,
    };

    std::size_t uniformBufferSize = renderer::Buffer::size(camera.uniformBuffer);

    auto mapping = renderer::Buffer::map(stagingBuffer, uniformBufferSize, stagingBufferOffset);

    std::memcpy(mapping.data.data(), matrices.data(), uniformBufferSize);

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferCopyRegion copyRegion = {
        .sourceOffsetBytes = stagingBufferOffset,
        .destinationOffsetBytes = 0,
        .sizeBytes = uniformBufferSize,
    };

    stagingBufferOffset += uniformBufferSize;

    renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, camera.uniformBuffer, {copyRegion});
}

void destroyCamera(Camera& camera) {
    if (camera.uniformBuffer) {
        renderer::Buffer::destroy(camera.uniformBuffer);
    }
}

void easeCameraTowards(Camera& camera, glm::vec3 position, float deltaTime) {
    float frameEase = 1.0f - std::pow(1.0f - camera.ease, deltaTime);
    glm::vec3 delta = position - camera.target;
    camera.target += delta * frameEase;
}
}