// #include <components/buttons.hpp>
// #include <components/entity_tags.hpp>
// #include <components/proxy.hpp>
// #include <components/tile.hpp>
// #include <components/transforms.hpp>
// #include <systems/buttons.hpp>
//
// // TEMPORARY, REPLACE IMMEDIATELY
// static constexpr float CORNER_FRAC = 0.25f;
// static constexpr glm::vec2 HOVER_DELTA = {0.20f, 0.20f};
// static constexpr glm::vec2 PULSE_DELTA = {0.15f, 0.15f};
// static constexpr float HOVER_DUR = 0.18f;
// static constexpr float PULSE_UP_DUR = 0.10f;
// static constexpr float REST_DUR = 0.18f;
//
// void systems::createButtons(entt::registry& registry, engine::Engine& engine) {
//     using namespace components;
//
//     for (auto entity : registry.view<Position, Scale, ButtonTag>()) {
//         auto pos = registry.get<Position>(entity).position;
//         auto scale = registry.get<Scale>(entity).scale;
//
//         auto& button = registry.emplace<Button>(entity);
//         registry.emplace<Original<Position>>(entity, pos);
//         registry.emplace<Original<Scale>>(entity, scale);
//
//         glm::vec2 corner = scale * CORNER_FRAC;
//         glm::vec2 center = scale - 2.0f * corner;
//
//         auto create_tile = [&](std::size_t idx, glm::vec2 tilePos, glm::vec2 tileS) {
//             entt::entity t = registry.create();
//             button.tiles[idx] = t;
//             registry.emplace<Proxy<TileInstance>>(t, engine.getTiles().size());
//             registry.emplace<Position>(t, tilePos);
//             registry.emplace<Scale>(t, tileS);
//             registry.emplace<TileTag>(t);
//
//             engine.getTiles().emplace_back();
//             engine.getButtonsTileCount()++;
//
//             auto& proxy = registry.get<Proxy<TileInstance>>(t);
//             auto& inst = engine.getTiles()[proxy.index];
//             inst.transform.position = glm::vec3{tilePos, 0.0f};
//             inst.transform.scale = tileS;
//
//             // minimal texture config (keep or remove)
//             inst.appearance.texture.sample.extent = {0.15f, 0.15f};
//             inst.appearance.texture.sample.position = {0.0375f, 0.0375f};
//             inst.appearance.texture.repeat = {1.0f, 1.0f};
//             inst.appearance.colourFactor = {1.0f, 1.0f, 1.0f};
//         };
//
//         glm::vec2 b = pos;
//
//         create_tile(0, b + glm::vec2{0.0f, 0.0f}, corner);
//         create_tile(1, b + glm::vec2{corner.x, 0.0f}, glm::vec2{center.x, corner.y});
//         create_tile(2, b + glm::vec2{corner.x + center.x, 0.0f}, corner);
//
//         create_tile(3, b + glm::vec2{0.0f, corner.y}, glm::vec2{corner.x, center.y});
//         create_tile(4, b + glm::vec2{corner.x, corner.y}, center);
//         create_tile(5, b + glm::vec2{corner.x + center.x, corner.y}, glm::vec2{corner.x, center.y});
//
//         create_tile(6, b + glm::vec2{0.0f, corner.y + center.y}, corner);
//         create_tile(7, b + glm::vec2{corner.x, corner.y + center.y}, glm::vec2{center.x, corner.y});
//         create_tile(8, b + glm::vec2{corner.x + center.x, corner.y + center.y}, corner);
//     }
// }
//
// static bool segmentIntersectsAABB(glm::vec2 a, glm::vec2 b, glm::vec2 min, glm::vec2 max) {
//
//     glm::vec2 d = b - a;
//
//     float tmin = 0.0f, tmax = 1.0f;
//
//     auto clip = [&](float p, float q) {
//         if (std::fabs(p) < 1e-9f) {
//             return q >= 0.0f;
//         }
//
//         float t = q / p;
//
//         if (p < 0.0f) {
//             if (t > tmax) {
//                 return false;
//             }
//             if (t > tmin) {
//                 tmin = t;
//             }
//         }
//         else {
//             if (t < tmin) {
//                 return false;
//             }
//             if (t < tmax) {
//                 tmax = t;
//             }
//         }
//
//         return tmin <= tmax;
//     };
//
//     if (!clip(-d.x, a.x - min.x) || !clip(d.x, max.x - a.x) || !clip(-d.y, a.y - min.y) || !clip(d.y, max.y - a.y)) {
//         return false;
//     }
//
//     return (tmin <= 1.0f && tmax >= 0.0f);
// }
//
// void systems::testButtons(entt::registry& registry, glm::vec2 mousePosition, glm::vec2 lastMousePosition, components::Bounds& bounds) {
//     using namespace components;
//     glm::vec2 mouse = mousePosition * bounds.scale * 0.5f;
//     glm::vec2 last = lastMousePosition * bounds.scale * 0.5f;
//
//     for (auto entity : registry.view<Position, Original<Position>, Scale, Original<Scale>, ButtonTag>()) {
//         auto curPos = registry.get<Position>(entity).position; // current visuals
//         auto curScale = registry.get<Scale>(entity).scale;
//         auto restPos = registry.get<Original<Position>>(entity).value.position;
//         auto restScale = registry.get<Original<Scale>>(entity).value.scale;
//
//         if (!registry.all_of<HoverState>(entity))
//             registry.emplace<HoverState>(entity);
//         auto& hover = registry.get<HoverState>(entity);
//
//         auto inside = [&](glm::vec2 p) {
//             return p.x > restPos.x && p.x < (restPos.x + restScale.x) &&
//                    p.y > restPos.y && p.y < (restPos.y + restScale.y);
//         };
//
//         bool wasInside = inside(last);
//         bool nowInside = inside(mouse);
//         bool segCross = segmentIntersectsAABB(last, mouse, restPos, restPos + restScale);
//
//         // ENTER
//         if (!hover.hovering && nowInside) {
//             glm::vec2 currentCenter = curPos + curScale * 0.5f;
//             auto& anim = registry.emplace_or_replace<ButtonAnimator>(entity);
//             anim.originalCentre = currentCenter;
//             anim.originalScale = curScale;
//             anim.targetScale = restScale + HOVER_DELTA;
//             anim.duration = HOVER_DUR;
//             anim.elapsed = 0.0f;
//             anim.pulseThenReturn = false;
//
//             auto& btn = registry.get<Button>(entity);
//
//             for (std::size_t i = 0; i < btn.tiles.size(); i++) {
//                 auto tpos = registry.get<Position>(btn.tiles[i]).position;
//                 auto ts = registry.get<Scale>(btn.tiles[i]).scale;
//                 anim.tileCenters[i] = tpos + ts * 0.5f;
//                 anim.tileScales[i] = ts;
//             }
//
//             hover.hovering = true;
//         }
//         // EXIT (normal)
//         else if (hover.hovering && !nowInside && !segCross) {
//             glm::vec2 currentCenter = curPos + curScale * 0.5f;
//             auto& anim = registry.emplace_or_replace<ButtonAnimator>(entity);
//
//             anim.originalCentre = currentCenter;
//             anim.originalScale = curScale;
//             anim.targetScale = restScale;
//             anim.duration = REST_DUR;
//             anim.elapsed = 0.0f;
//             anim.pulseThenReturn = false;
//
//             auto& btn = registry.get<Button>(entity);
//
//             for (std::size_t i = 0; i < btn.tiles.size(); i++) {
//                 auto tpos = registry.get<Position>(btn.tiles[i]).position;
//                 auto ts = registry.get<Scale>(btn.tiles[i]).scale;
//                 anim.tileCenters[i] = tpos + ts * 0.5f;
//                 anim.tileScales[i] = ts;
//             }
//
//             hover.hovering = false;
//         }
//
//         else if (!hover.hovering && !nowInside && !wasInside && segCross) {
//             glm::vec2 currentCenter = curPos + curScale * 0.5f;
//             auto& anim = registry.emplace_or_replace<ButtonAnimator>(entity);
//
//             anim.originalCentre = currentCenter;
//             anim.originalScale = curScale;
//             anim.targetScale = restScale + PULSE_DELTA;
//             anim.duration = PULSE_UP_DUR;
//             anim.elapsed = 0.0f;
//             anim.pulseThenReturn = true;
//
//             auto& btn = registry.get<Button>(entity);
//
//             for (std::size_t i = 0; i < btn.tiles.size(); i++) {
//                 auto tpos = registry.get<Position>(btn.tiles[i]).position;
//                 auto ts = registry.get<Scale>(btn.tiles[i]).scale;
//                 anim.tileCenters[i] = tpos + ts * 0.5f;
//                 anim.tileScales[i] = ts;
//             }
//
//             hover.hovering = false;
//         }
//     }
// }
//
// void systems::animateButtons(entt::registry& registry, float deltaTime) {
//     using namespace components;
//     std::vector<entt::entity> finished;
//
//     for (auto entity : registry.view<Position, Scale, Button, ButtonAnimator, ButtonTag>()) {
//         auto& position = registry.get<Position>(entity).position;
//         auto& scale = registry.get<Scale>(entity).scale;
//         auto& button = registry.get<Button>(entity);
//         auto& anim = registry.get<ButtonAnimator>(entity);
//
//         anim.elapsed += deltaTime;
//         float t = std::clamp(anim.elapsed / anim.duration, 0.0f, 1.0f);
//         t = t * t * (3.0f - 2.0f * t); // smoothstep
//
//         glm::vec2 newScale = glm::mix(anim.originalScale, anim.targetScale, t);
//         glm::vec2 startCenter = anim.originalCentre;
//         glm::vec2 currentCenter = startCenter; // anchor kept fixed for simplicity
//
//         position = currentCenter - newScale * 0.5f;
//         scale = newScale;
//
//         glm::vec2 safeOrig = anim.originalScale;
//         const float eps = 1e-6f;
//         if (safeOrig.x < eps)
//             safeOrig.x = eps;
//         if (safeOrig.y < eps)
//             safeOrig.y = eps;
//         glm::vec2 ratio = newScale / safeOrig;
//
//         // update tile ECS components (use cached originals, no compounding)
//         for (std::size_t i = 0; i < button.tiles.size() && i < anim.tileCenters.size(); ++i) {
//             entt::entity tile = button.tiles[i];
//             glm::vec2 origTileCenter = anim.tileCenters[i];
//             glm::vec2 origTileScale = anim.tileScales[i];
//
//             glm::vec2 rel = origTileCenter - startCenter;
//             glm::vec2 newTileCenter = currentCenter + rel * ratio;
//             glm::vec2 newTileScale = origTileScale * ratio;
//             glm::vec2 newTilePos = newTileCenter - newTileScale * 0.5f;
//
//             if (registry.all_of<Position>(tile))
//                 registry.get<Position>(tile).position = newTilePos;
//             if (registry.all_of<Scale>(tile))
//                 registry.get<Scale>(tile).scale = newTileScale;
//         }
//
//         if (t >= 1.0f) {
//             if (anim.pulseThenReturn) {
//                 // start return animator sampled from current visual state
//                 glm::vec2 currentCenterVis = position + scale * 0.5f;
//                 glm::vec2 currentScaleVis = scale;
//                 glm::vec2 restScale = currentScaleVis;
//                 if (registry.all_of<Original<Scale>>(entity))
//                     restScale = registry.get<Original<Scale>>(entity).value.scale;
//
//                 auto& second = registry.emplace_or_replace<ButtonAnimator>(entity);
//                 second.originalCentre = currentCenterVis;
//                 second.originalScale = currentScaleVis;
//                 second.targetScale = restScale;
//                 second.duration = REST_DUR;
//                 second.elapsed = 0.0f;
//                 second.pulseThenReturn = false;
//
//                 for (std::size_t i = 0; i < button.tiles.size(); i++) {
//                     auto tpos = registry.get<Position>(button.tiles[i]).position;
//                     auto ts = registry.get<Scale>(button.tiles[i]).scale;
//                     anim.tileCenters[i] = tpos + ts * 0.5f;
//                     anim.tileScales[i] = ts;
//                 }
//
//                 continue; // new animator is running; don't remove yet
//             }
//
//             // final snap and remove animator
//             scale = anim.targetScale;
//             position = anim.originalCentre - scale * 0.5f;
//             finished.push_back(entity);
//         }
//     }
//
//     for (auto e : finished) {
//         if (registry.valid(e) && registry.all_of<ButtonAnimator>(e))
//             registry.remove<ButtonAnimator>(e);
//     }
// }