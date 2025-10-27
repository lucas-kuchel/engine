#include <components/camera.hpp>
#include <components/entity_tags.hpp>
#include <components/transforms.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <systems/camera.hpp>

#include <cstring>

void systems::updateCameraScales(entt::registry& registry, glm::vec2 scale) {
    for (auto& entity : registry.view<components::Camera, components::Scale>()) {
        auto& cameraScale = registry.get<components::Scale>(entity);

        cameraScale.scale = scale;
    }
}

void systems::animateCameras(entt::registry& registry, float deltaTime) {
    std::vector<entt::entity> scaleAnimationsFinished;
    std::vector<entt::entity> positionAnimationsFinished;

    for (auto& entity : registry.view<components::Camera, components::CameraScaleAnimator>()) {
        auto& camera = registry.get<components::Camera>(entity);
        auto& animator = registry.get<components::CameraScaleAnimator>(entity);

        animator.timeElapsed += deltaTime;

        float t = std::clamp(animator.timeElapsed / animator.duration, 0.0f, 1.0f);

        float smoothT = t * t * t * (t * (t * 6 - 15) + 10);

        camera.scale = glm::mix(animator.startScale, animator.targetScale, smoothT);

        if (t >= 1.0f) {
            camera.scale = animator.targetScale;
            scaleAnimationsFinished.push_back(entity);
        }
    }

    for (auto entity : registry.view<components::Camera, components::Position, components::CameraPositionAnimator>()) {
        auto& position = registry.get<components::Position>(entity);
        auto& animator = registry.get<components::CameraPositionAnimator>(entity);

        animator.timeElapsed += deltaTime;

        float t = std::clamp(animator.timeElapsed / animator.duration, 0.0f, 1.0f);

        float smoothT = t * t * t * (t * (t * 6 - 15) + 10);

        position.position = glm::vec3(glm::mix(animator.startPosition, animator.targetPosition, smoothT), 0.0f);

        if (t >= 1.0f) {
            position.position = glm::vec3(animator.targetPosition, 0.0f);
            positionAnimationsFinished.push_back(entity);
        }
    }

    for (auto& entity : scaleAnimationsFinished) {
        registry.remove<components::CameraScaleAnimator>(entity);
    }

    for (auto& entity : positionAnimationsFinished) {
        registry.remove<components::CameraPositionAnimator>(entity);
    }
}

void systems::createCameras(entt::registry& registry, renderer::Device& device) {
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

void systems::updateCameras(entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::uint64_t& stagingBufferOffset) {
    if (!stagingBuffer) {
        return;
    }

    for (auto& entity : registry.view<components::CameraBuffer>()) {
        components::CameraUploadData uploadData = {};

        if (registry.all_of<components::Camera, components::Position, components::Scale, components::Bounds>(entity)) {
            auto& camera = registry.get<components::Camera>(entity);
            auto& scale = registry.get<components::Scale>(entity);
            auto& position = registry.get<components::Position>(entity);
            auto& bounds = registry.get<components::Bounds>(entity);

            uploadData.projection = {1.0f};
            uploadData.view = {1.0f};

            float rotation = 0.0f;

            if (registry.all_of<components::Rotation>(entity)) {
                rotation = registry.get<components::Rotation>(entity).angle;
            }

            float aspect = scale.scale.x / scale.scale.y;
            float halfHeight = camera.scale;
            float halfWidth = camera.scale * aspect;

            float left = -halfWidth;
            float right = halfWidth;
            float bottom = -halfHeight;
            float top = halfHeight;

            bounds.scale.x = halfWidth * 2.0f;
            bounds.scale.y = halfHeight * 2.0f;

            uploadData.projection = glm::orthoRH_ZO(left, right, bottom, top, camera.near, camera.far);
            uploadData.projection[1][1] *= -1.0f;

            uploadData.view = glm::rotate(uploadData.view, glm::radians(rotation), glm::vec3{0.0f, 0.0f, 1.0f});
            uploadData.view = glm::translate(uploadData.view, glm::vec3{-position.position, 0.0f});
        }

        auto& buffer = registry.get<components::CameraBuffer>(entity);

        if (!buffer.buffer) {
            continue;
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

void systems::destroyCameras(entt::registry& registry) {
    for (auto& entity : registry.view<components::CameraBuffer>()) {

        auto& buffer = registry.get<components::CameraBuffer>(entity);
        if (!buffer.buffer) {
            return;
        }

        renderer::Buffer::destroy(buffer.buffer);
    }
}