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
        float minExtent = static_cast<float>(std::min(camera.extent.x, camera.extent.y));

        float halfSize = camera.scale * 0.5f;

        float halfWidth = halfSize * (static_cast<float>(camera.extent.x) / minExtent);
        float halfHeight = halfSize * (static_cast<float>(camera.extent.y) / minExtent);

        float left = -halfWidth;
        float right = +halfWidth;
        float bottom = -halfHeight;
        float top = +halfHeight;

        camera.projection = glm::orthoZO(left, right, bottom, top, 0.0f, 1.0f);

        camera.projection[1][1] *= -1;

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(-camera.position, 0.0f));
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), camera.rotation, glm::vec3(0.0f, 0.0f, 1.0f));

        camera.view = rotation * translation;

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

    void easeCameraTowards(Camera& camera, glm::vec2 position, float deltaTime) {
        float frameEase = 1.0f - std::pow(1.0f - camera.ease, deltaTime);
        glm::vec2 delta = position - camera.position;
        camera.position += delta * frameEase;
    }
}