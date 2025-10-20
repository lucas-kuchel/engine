// #include <game/map.hpp>
// #include <game/mesh.hpp>
// #include <game/tags.hpp>
// #include <game/transforms.hpp>
//
// #include <fstream>
//
// #include <nlohmann/json.hpp>
//
// #include <glm/gtc/matrix_transform.hpp>
//
// namespace game {
//     void loadMap(entt::registry& registry, const std::string& filepath, std::uint64_t& tileCount) {
//         std::ifstream mapFile(filepath);
//         std::string mapFileContents(std::istreambuf_iterator<char>{mapFile}, {});
//         nlohmann::json mapFileJson = nlohmann::json::parse(mapFileContents);
//
//         if (mapFileJson.contains("tiles") && mapFileJson["tiles"].is_array()) {
//             for (const auto& tileJson : mapFileJson["tiles"]) {
//                 auto tile = registry.create();
//                 auto& tilePosition = registry.emplace<Position>(tile);
//                 auto& tileScale = registry.emplace<Scale>(tile);
//                 auto& tileRotation = registry.emplace<Rotation>(tile);
//                 auto& tileTexture = registry.emplace<MeshTexture>(tile);
//
//                 registry.emplace<MeshTransform>(tile);
//                 registry.emplace<StaticTileTag>(tile);
//
//                 tileCount++;
//
//                 if (tileJson.contains("transform") && tileJson["transform"].is_object()) {
//                     const auto& tileEntry = tileJson["transform"];
//
//                     if (tileEntry.contains("position")) {
//                         auto position = tileEntry["position"];
//
//                         tilePosition.position.x = position.size() > 0 ? position[0].get<float>() : 0.0f;
//                         tilePosition.position.y = position.size() > 1 ? position[1].get<float>() : 0.0f;
//                         tilePosition.position.z = position.size() > 2 ? position[2].get<float>() : 0.0f;
//                     }
//
//                     if (tileEntry.contains("scale")) {
//                         auto scale = tileEntry["scale"];
//
//                         tileScale.scale.x = scale.size() > 0 ? scale[0].get<float>() : 1.0f;
//                         tileScale.scale.y = scale.size() > 1 ? scale[1].get<float>() : 1.0f;
//                     }
//
//                     if (tileEntry.contains("rotation")) {
//                         auto rotation = tileEntry["rotation"];
//
//                         tileRotation.angle = rotation.get<float>();
//                     }
//                 }
//
//                 if (tileJson.contains("texture") && tileJson["texture"].is_object()) {
//                     const auto& texture = tileJson["texture"];
//
//                     if (texture.contains("position")) {
//                         auto position = texture["position"];
//
//                         tileTexture.position.x = position.size() > 0 ? position[0].get<float>() : 0.0f;
//                         tileTexture.position.y = position.size() > 1 ? position[1].get<float>() : 0.0f;
//                     }
//
//                     if (texture.contains("extent")) {
//                         auto extent = texture["extent"];
//
//                         tileTexture.extent.x = extent.size() > 0 ? extent[0].get<float>() : 0.0f;
//                         tileTexture.extent.y = extent.size() > 1 ? extent[1].get<float>() : 0.0f;
//                     }
//
//                     if (texture.contains("offset")) {
//                         auto offset = texture["offset"];
//
//                         tileTexture.offset.x = offset.size() > 0 ? offset[0].get<float>() : 0.0f;
//                         tileTexture.offset.y = offset.size() > 1 ? offset[1].get<float>() : 0.0f;
//                     }
//
//                     if (texture.contains("scale")) {
//                         auto scale = texture["scale"];
//
//                         tileTexture.scale.x = scale.size() > 0 ? scale[0].get<float>() : 1.0f;
//                         tileTexture.scale.y = scale.size() > 1 ? scale[1].get<float>() : 1.0f;
//                     }
//                 }
//             }
//         }
//     }
// }