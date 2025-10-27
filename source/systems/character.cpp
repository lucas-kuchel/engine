#include <components/camera.hpp>
#include <components/character.hpp>
#include <components/entity_tags.hpp>
#include <components/transforms.hpp>
#include <systems/character.hpp>

void systems::cameraFollowCharacter(entt::registry& registry, entt::entity& character, entt::entity& camera, glm::vec2 mousePosition, float deltaTime) {
    if (!registry.all_of<components::Position, components::Character, components::ActiveCharacterTag>(character) ||
        !registry.all_of<components::Position, components::Camera, components::ActiveCameraTag>(camera)) {
        return;
    }

    auto& cameraPosition = registry.get<components::Position>(camera);
    auto& characterPosition = registry.get<components::Position>(character);
    auto& cameraComponent = registry.get<components::Camera>(camera);

    if (cameraComponent.mode != components::CameraMode::FOLLOW) {
        return;
    }

    auto target = characterPosition.position + mousePosition;
    auto direction = target - cameraPosition.position;
    cameraPosition.position += direction * deltaTime * 4.0f;
}