#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/settings.hpp>

#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/queue.hpp>

#include <array>
#include <cstring>
#include <iostream>

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

        std::cout << "extent: " << camera.extent.x << "x" << camera.extent.y
                  << " aspect=" << aspect << "\n";

        camera.projection = glm::perspective(glm::radians(camera.fov), aspect, camera.near, camera.far);
        camera.projection[1][1] *= -1.0f;

        float pitch = glm::radians(camera.rotation.x);
        float yaw = glm::radians(camera.rotation.y);

        camera.orientation.x = std::cos(yaw) * std::cos(pitch);
        camera.orientation.y = std::sin(pitch);
        camera.orientation.z = std::sin(yaw) * std::cos(pitch);

        glm::vec3 forward = glm::normalize(camera.orientation);
        glm::vec3 target = camera.position + forward;

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        camera.view = glm::lookAt(camera.position, target, up);

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