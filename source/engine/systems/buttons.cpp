#include <engine/components/buttons.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/tile.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/buttons.hpp>

void engine::systems::createButtons(entt::registry& registry) {
    using namespace components;

    for (auto& entity : registry.view<Position, Scale, ButtonTag>()) {
        auto& position = registry.get<Position>(entity).position;
        auto& scale = registry.get<Scale>(entity).scale;

        auto& button = registry.emplace<Button>(entity);
        registry.emplace<Original<Position>>(entity, position);
        registry.emplace<Original<Scale>>(entity, scale);

        for (auto& tile : button.tiles) {
            registry.emplace<TileInstance>(tile);
            registry.emplace<Position>(tile);
            registry.emplace<Scale>(tile);
            registry.emplace<TileTag>(tile);

            // tile bounds must fit in button bounds
        }
    }
}

void engine::systems::testButtons(entt::registry& registry, glm::vec2 mousePosition, glm::vec2 lastMousePosition) {
    using namespace components;

    for (auto& entity : registry.view<Position, Original<Position>, Scale, Original<Scale>, ButtonTag>()) {
        auto& position = registry.get<Position>(entity).position;
        auto& scale = registry.get<Scale>(entity).scale;
        auto& originalPosition = registry.get<Original<Position>>(entity).value.position;
        auto& originalScale = registry.get<Original<Scale>>(entity).value.scale;

        auto collides = [&](glm::vec2& mousePos) {
            glm::vec2 entityMin = position;
            glm::vec2 entityMax = position + scale;

            return (entityMax.x > mousePos.x) && (entityMin.x < mousePos.x) &&
                   (entityMax.y > mousePos.y) && (entityMin.y < mousePos.y);
        };

        bool isColliding = collides(mousePosition);
        bool wasColliding = collides(lastMousePosition);

        if (wasColliding && !isColliding) {
            auto& animator = registry.emplace_or_replace<ButtonAnimator>(entity);

            animator.originalScale = scale;
            animator.targetScale = originalScale;
            animator.originalPosition = originalPosition;
            animator.cachedPosition = position;
            animator.duration = 1.0f;
            animator.elapsed = 0.0f;
        }
        else if (!wasColliding && isColliding) {
            auto& animator = registry.emplace_or_replace<ButtonAnimator>(entity);

            animator.originalScale = originalScale;
            animator.targetScale = scale + glm::vec2{0.2, 0.2};
            animator.originalPosition = position;
            animator.cachedPosition = originalPosition;
            animator.duration = 1.0f;
            animator.elapsed = 0.0f;
        }
    }
}

void engine::systems::animateButtons(entt::registry& registry, float deltaTime) {
    using namespace components;

    std::vector<entt::entity> animationsFinished;
    animationsFinished.reserve(16);

    for (auto entity : registry.view<Position, Scale, Button, ButtonAnimator, ButtonTag>()) {
        auto& button = registry.get<Button>(entity);
        auto& position = registry.get<Position>(entity).position; // bottom-left
        auto& scale = registry.get<Scale>(entity).scale;
        auto& animator = registry.get<ButtonAnimator>(entity);

        animator.elapsed += deltaTime;
        float t = std::clamp(animator.elapsed / animator.duration, 0.0f, 1.0f);

        // Optional: ease in/out for smoother look
        t = t * t * (3.0f - 2.0f * t);

        // Compute current center before scaling
        glm::vec2 center = position + scale * 0.5f;

        // Interpolate scale
        glm::vec2 newScale = glm::mix(animator.originalScale, animator.targetScale, t);

        // Recompute bottom-left position so center stays constant
        glm::vec2 newPosition = center - newScale * 0.5f;

        // Apply
        position = newPosition;
        scale = newScale;

        // Animate button tiles (child visuals)
        for (auto tileEntity : button.tiles) {
            if (!registry.valid(tileEntity) ||
                !registry.all_of<Position, Scale, TileTag>(tileEntity)) {
                continue;
            }

            auto& tilePos = registry.get<Position>(tileEntity).position;
            auto& tileScale = registry.get<Scale>(tileEntity).scale;

            // Maintain each tile's relative offset to button center
            glm::vec2 relOffset = (tilePos + tileScale * 0.5f) - center;
            glm::vec2 newTileCenter = center + relOffset * (newScale / animator.originalScale);
            glm::vec2 newTilePos = newTileCenter - (tileScale * 0.5f);

            tilePos = newTilePos;
            // You could optionally scale tiles slightly as well if desired
            // tileScale = tileScale * (newScale / animator.originalScale);
        }

        if (t >= 1.0f)
            animationsFinished.push_back(entity);
    }

    // Remove completed animators safely after iteration
    for (auto entity : animationsFinished) {
        if (registry.valid(entity) && registry.all_of<ButtonAnimator>(entity)) {
            registry.remove<ButtonAnimator>(entity);
        }
    }
}