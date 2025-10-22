#include <engine/components/camera.hpp>
#include <engine/components/character.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/character.hpp>

void engine::systems::cameraFollowCharacter(entt::registry& registry, entt::entity& character, entt::entity& camera, float deltaTime) {
    if (!registry.all_of<components::Position, components::Character, components::ActiveCharacterTag>(character) ||
        !registry.all_of<components::Camera, components::ActiveCameraTag, components::Position>(camera)) {
        return;
    }

    auto& cameraPosition = registry.get<components::Position>(camera);
    auto& characterPosition = registry.get<components::Position>(character);
    auto& cameraComponent = registry.get<components::Camera>(camera);

    if (cameraComponent.mode != components::CameraMode::FOLLOW) {
        return;
    }

    auto direction = characterPosition.position - cameraPosition.position;
    cameraPosition.position += glm::vec3(direction.x, direction.y, 0.0f) * deltaTime * 4.0f;
}