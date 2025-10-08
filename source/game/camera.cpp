#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/settings.hpp>

#include <renderer/resources/fence.hpp>

#include <renderer/device.hpp>
#include <renderer/queue.hpp>

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

        camera.uniformBuffer = data::makeUnique<renderer::Buffer>(bufferCreateInfo);

        auto stagingBufferData = stagingBuffer.map(camera.uniformBuffer->size(), stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), matrices.data(), camera.uniformBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion bufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = camera.uniformBuffer->size(),
        };

        stagingBufferOffset += camera.uniformBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, camera.uniformBuffer.ref(), {bufferCopyRegion});
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

        auto stagingBufferData = stagingBuffer.map(camera.uniformBuffer->size(), stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), matrices.data(), camera.uniformBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion bufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = camera.uniformBuffer->size(),
        };

        stagingBufferOffset += camera.uniformBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, camera.uniformBuffer.ref(), {bufferCopyRegion});
    }

    void easeCameraTowards(Camera& camera, glm::vec2& position, float deltaTime) {
        glm::vec2 delta = position - camera.position;
        camera.position += delta * camera.ease * deltaTime;
    }
}