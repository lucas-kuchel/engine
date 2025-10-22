#include <engine/components/camera.hpp>
#include <engine/components/character.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/character.hpp>

void engine::systems::cameraFollowCharacter(entt::registry& registry, entt::entity& character, entt::entity& camera, float deltaTime) {
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

    auto direction = characterPosition.position - cameraPosition.position;
    cameraPosition.position += direction * deltaTime * 4.0f;
}