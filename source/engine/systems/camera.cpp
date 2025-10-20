#include <engine/components/camera.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/camera.hpp>

#include <cstring>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

void engine::systems::createCameras(entt::registry& registry, renderer::Device& device) {
    for (auto& entity : registry.view<components::CameraBuffer>()) {
        auto& buffer = registry.get<components::CameraBuffer>(entity);

        if (buffer.buffer) {
            continue;
        }

        renderer::BufferCreateInfo bufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = sizeof(components::CameraUploadData),
        };

        buffer.buffer = renderer::Buffer::create(bufferCreateInfo);
    }
}

void engine::systems::updateCameras(entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::uint64_t& stagingBufferOffset) {
    if (!stagingBuffer) {
        return;
    }

    for (auto& entity : registry.view<components::CameraBuffer>()) {
        auto& buffer = registry.get<components::CameraBuffer>(entity);

        if (!buffer.buffer) {
            continue;
        }

        components::CameraUploadData uploadData = {};

        if (registry.all_of<components::Camera>(entity)) {
            auto& camera = registry.get<components::Camera>(entity);

            glm::vec3 position = {0.0f, 0.0f, 0.0f};
            glm::vec2 scale = {1.0f, 1.0f};
            float rotation = 0.0f;

            if (registry.all_of<components::Position>(entity)) {
                position = registry.get<components::Position>(entity).position;
            }

            if (registry.all_of<components::Scale>(entity)) {
                scale = registry.get<components::Scale>(entity).scale;
            }

            if (registry.all_of<components::Rotation>(entity)) {
                rotation = registry.get<components::Rotation>(entity).angle;
            }

            bool widthIsLonger = scale.x >= scale.y;

            float halfShort = camera.scale;
            float halfWidth, halfHeight;

            if (widthIsLonger) {
                halfWidth = halfShort * (scale.x / scale.y);
                halfHeight = halfShort;
            }
            else {
                halfWidth = halfShort;
                halfHeight = halfShort * (scale.y / scale.x);
            }

            float left = -halfWidth;
            float right = halfWidth;
            float bottom = -halfHeight;
            float top = halfHeight;

            glm::quat quaternion = {glm::radians(glm::vec3{0.0f, 0.0f, rotation})};
            glm::vec3 forward = quaternion * glm::vec3{0.0f, 0.0f, -1.0f};
            glm::vec3 target = position + forward;

            uploadData.projection = glm::orthoRH_ZO(left, right, bottom, top, camera.near, camera.far);
            uploadData.projection[1][1] *= -1.0f;
            uploadData.view = glm::lookAt(position, target, glm::vec3{0.0f, 1.0f, 0.0f});
        }

        auto mapping = renderer::Buffer::map(stagingBuffer, renderer::Buffer::size(buffer.buffer), stagingBufferOffset);

        std::memcpy(mapping.data.data(), &uploadData, sizeof(components::CameraUploadData));

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion copyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = sizeof(components::CameraUploadData),
        };

        stagingBufferOffset += copyRegion.sizeBytes;

        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, buffer.buffer, {copyRegion});
    }
}

void engine::systems::destroyCameras(entt::registry& registry) {
    for (auto& entity : registry.view<components::CameraBuffer>()) {

        auto& buffer = registry.get<components::CameraBuffer>(entity);
        if (!buffer.buffer) {
            return;
        }

        renderer::Buffer::destroy(buffer.buffer);
    }
}