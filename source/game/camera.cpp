#include <game/camera.hpp>
#include <game/settings.hpp>
#include <game/tags.hpp>
#include <game/transforms.hpp>

#include <array>
#include <cstring>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace game {
    void createCameraBuffers(entt::registry& registry, renderer::Device& device) {
        for (auto& entity : registry.view<CameraBuffer, CameraTag>()) {
            auto& cameraBuffer = registry.get<CameraBuffer>(entity);

            if (cameraBuffer.buffer) {
                continue;
            }

            renderer::BufferCreateInfo bufferCreateInfo = {
                .device = device,
                .memoryType = renderer::MemoryType::DEVICE_LOCAL,
                .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
                .sizeBytes = 2 * sizeof(glm::mat4),
            };
            cameraBuffer.buffer = renderer::Buffer::create(bufferCreateInfo);
        }
    }

    void updateCameraBuffers(entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t& stagingBufferOffset) {
        if (!stagingBuffer) {
            return;
        }

        for (auto& entity : registry.view<CameraBuffer, Projection, View, CameraTag>()) {
            auto& cameraBuffer = registry.get<CameraBuffer>(entity);
            auto& projection = registry.get<Projection>(entity);
            auto& view = registry.get<View>(entity);

            if (!cameraBuffer.buffer) {
                continue;
            }

            std::array<glm::mat4, 2> data = {
                projection.matrix,
                view.matrix,
            };

            auto mapping = renderer::Buffer::map(stagingBuffer, renderer::Buffer::size(cameraBuffer.buffer), stagingBufferOffset);

            std::memcpy(mapping.data.data(), data.data(), sizeof(data));

            renderer::Buffer::unmap(stagingBuffer, mapping);

            renderer::BufferCopyRegion copyRegion = {
                .sourceOffsetBytes = stagingBufferOffset,
                .destinationOffsetBytes = 0,
                .sizeBytes = sizeof(data),
            };

            stagingBufferOffset += copyRegion.sizeBytes;

            renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, cameraBuffer.buffer, {copyRegion});
        }
    }

    void destroyCameraBuffers(entt::registry& registry) {
        for (auto& entity : registry.view<CameraBuffer, CameraTag>()) {
            auto& cameraBuffer = registry.get<CameraBuffer>(entity);

            if (!cameraBuffer.buffer) {
                continue;
            }

            renderer::Buffer::destroy(cameraBuffer.buffer);
        }
    }

    void cameraFollow(entt::registry& registry) {
        for (auto& entity : registry.view<CameraTag, Position, Target>()) {
            auto& position = registry.get<Position>(entity);
            auto& target = registry.get<Target>(entity);

            if (!registry.all_of<Position>(target.handle)) {
                continue;
            }

            auto& targetPosition = registry.get<Position>(target.handle);

            position.position = targetPosition.position + glm::vec3{0.0, 5.0, 15.0};
        }
    }

    void updateCameraPerspectives(entt::registry& registry, glm::vec2 extent) {
        for (auto& entity : registry.view<Camera, Projection, Perspective, CameraTag>()) {
            auto& camera = registry.get<Camera>(entity);
            auto& projection = registry.get<Projection>(entity);
            auto& perspective = registry.get<Perspective>(entity);

            perspective.aspectRatio = extent.x / extent.y;

            projection.matrix = glm::perspectiveRH_ZO(glm::radians(perspective.fov), perspective.aspectRatio, camera.near, camera.far);
            projection.matrix[1][1] *= -1.0f;
        }
    }

    void updateCameraOrthographics(entt::registry& registry) {
        for (auto& entity : registry.view<Camera, Projection, Orthographic, CameraTag>()) {
            auto& camera = registry.get<Camera>(entity);
            auto& projection = registry.get<Projection>(entity);
            auto& orthographic = registry.get<Orthographic>(entity);

            float halfWidth = (orthographic.right - orthographic.left) * 0.5f / orthographic.scale;
            float halfHeight = (orthographic.top - orthographic.bottom) * 0.5f / orthographic.scale;
            float centreX = (orthographic.right + orthographic.left) * 0.5f;
            float centreY = (orthographic.top + orthographic.bottom) * 0.5f;

            projection.matrix = glm::orthoRH_ZO(centreX - halfWidth, centreX + halfWidth, centreY - halfHeight, centreY + halfHeight, camera.near, camera.far);
            projection.matrix[1][1] *= -1.0f;
        }
    }

    void updateCameraViews(entt::registry& registry) {
        for (auto& entity : registry.view<View, Position, Rotation, CameraTag>()) {
            auto& view = registry.get<View>(entity);
            auto& position = registry.get<Position>(entity);
            auto& rotation = registry.get<Rotation>(entity);

            glm::quat quaternion = {glm::radians(rotation.rotation)};

            glm::vec3 forward = quaternion * glm::vec3{0.0f, 0.0f, -1.0f};
            glm::vec3 target = position.position + forward;

            view.matrix = glm::lookAt(position.position, target, glm::vec3{0.0f, 1.0f, 0.0f});
        }
    }
}