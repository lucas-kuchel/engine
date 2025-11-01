#include <components/camera.hpp>
#include <components/tags.hpp>
#include <components/transforms.hpp>

#include <engine/engine.hpp>

#include <systems/camera.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstring>

void systems::cameras::calculateCameraData(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();

    auto view = registry.view<Camera, Scale, Position, CameraData>();

    for (auto [entity, camera, scale, position, data] : view.each()) {
        data.projection = {1.0f};
        data.view = {1.0f};

        float aspect = scale.scale.x / scale.scale.y;
        float halfHeight = camera.size;
        float halfWidth = camera.size * aspect;

        float left = -halfWidth;
        float right = halfWidth;
        float bottom = -halfHeight;
        float top = halfHeight;

        data.projection = glm::orthoRH_ZO(left, right, bottom, top, camera.near, camera.far);
        data.view = glm::translate(data.view, glm::vec3{-position.position, 0.0f});

        data.projection[1][1] *= -1.0f;
    }
}

void systems::cameras::animateCameraSizes(engine::Engine& engine) {
    using namespace components;

    std::vector<entt::entity> animationsFinished;

    auto& registry = engine.getRegistry();

    auto deltaTime = engine.getDeltaTime();
    auto view = registry.view<Camera, CameraSizeAnimator>();

    for (auto [entity, camera, animator] : view.each()) {
        animator.timeElapsed += deltaTime;

        float t = std::clamp(animator.timeElapsed / animator.duration, 0.0f, 1.0f);

        float smoothT = t * t * t * (t * (t * 6 - 15) + 10);

        camera.size = glm::mix(animator.startSize, animator.targetSize, smoothT);

        if (t >= 1.0f) {
            camera.size = animator.targetSize;
            animationsFinished.push_back(entity);
        }
    }

    for (auto& entity : animationsFinished) {
        registry.remove<components::CameraSizeAnimator>(entity);
    }
}

void systems::cameras::animateCameraPositions(engine::Engine& engine) {
    using namespace components;

    std::vector<entt::entity> animationsFinished;

    auto& registry = engine.getRegistry();

    auto deltaTime = engine.getDeltaTime();
    auto view = registry.view<Camera, Position, CameraPositionAnimator>();

    for (auto [entity, camera, position, animator] : view.each()) {
        animator.timeElapsed += deltaTime;

        float t = std::clamp(animator.timeElapsed / animator.duration, 0.0f, 1.0f);

        float smoothT = t * t * t * (t * (t * 6 - 15) + 10);

        position.position = glm::vec3(glm::mix(animator.startPosition, animator.targetPosition, smoothT), 0.0f);

        if (t >= 1.0f) {
            position.position = glm::vec3(animator.targetPosition, 0.0f);
            animationsFinished.push_back(entity);
        }
    }

    for (auto& entity : animationsFinished) {
        registry.remove<components::CameraPositionAnimator>(entity);
    }
}

void systems::cameras::makeCamerasFollowTarget(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();

    auto deltaTime = engine.getDeltaTime();
    auto view = registry.view<Camera, Position, CameraTarget>();

    for (auto [entity, camera, position, target] : view.each()) {
        auto& targetPosition = registry.get<Position>(target.target);
        auto direction = targetPosition.position - position.position;

        if (glm::length(direction) < 0.0001f) {
            position.position = targetPosition.position;
        }
        else {
            position.position += direction * deltaTime;
        }
    }
}

void systems::cameras::uploadCameraData(engine::Engine& engine) {
    using namespace components;

    auto cameraEntity = engine.getCurrentCamera();

    auto& registry = engine.getRegistry();

    if (!registry.all_of<CameraData>(cameraEntity)) {
        return;
    }

    auto& cameraData = registry.get<CameraData>(cameraEntity);
    auto& stagingManager = engine.getStagingManager();
    auto& transferBuffer = engine.getTransferBuffer();
    auto& cameraBuffer = engine.getCameraBuffer();
    auto& stagingBuffer = stagingManager.getCurrentBuffer();
    auto& stagingOffset = stagingManager.getOffset();

    auto mapping = renderer::Buffer::map(stagingBuffer, sizeof(cameraData), stagingOffset);

    std::memcpy(mapping.data.data(), &cameraData, sizeof(cameraData));

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferCopyRegion copyRegion = {
        .sourceOffsetBytes = stagingOffset,
        .destinationOffsetBytes = 0,
        .sizeBytes = sizeof(cameraData),
    };

    renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, cameraBuffer, {copyRegion});

    stagingOffset += sizeof(cameraData);
}