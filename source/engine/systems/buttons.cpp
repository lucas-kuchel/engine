#include <engine/components/entity_tags.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/buttons.hpp>

// void engine::systems::updateButtons(entt::registry& registry, glm::vec2 mousePosition, glm::vec2 cameraExtent, float deltaTime) {
//     using namespace components;
//
//     for (auto& entity : registry.view<Position, Scale, Last<Scale>, ButtonTag>()) {
//         auto& position = registry.get<Position>(entity).position;
//         auto& scale = registry.get<Scale>(entity).scale;
//         auto& originalScale = registry.get<Last<Scale>>(entity).value;
//
//         auto centrePosition = position + (scale * 0.5f);
//         auto mouseWorldPosition = mousePosition * cameraExtent;
//
//         // position is bottom left, scale extends up and right of that
//         // if mousePosition (already in NDC, x: [-1, 1], y: [-1, 1]) intersects button (position is in world space), scale button up
//         // else scale button back to regular size (originalScale is the original scale)
//     }
// }