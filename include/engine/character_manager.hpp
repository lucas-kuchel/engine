#pragma once

#include <components/character.hpp>
#include <components/controllers.hpp>
#include <components/tile.hpp>
#include <entt/entt.hpp>

namespace engine {
    struct CharacterData {
        ::components::Position* position = nullptr;
        ::components::Scale* scale = nullptr;
        ::components::TileInstance* tileInstance = nullptr;
        ::components::TileProxy* tileProxy = nullptr;
        ::components::Speed* speed = nullptr;
        ::components::PositionController* controller = nullptr;
    };

    struct CharacterDescriptor {
        ::components::TileInstance::Appearance appearance;
        ::components::Position position;
        ::components::Scale scale;
    };

    class CharacterManager {
    public:
        CharacterData insert(const CharacterDescriptor& descriptor);
        CharacterData getCurrentCharacter();
    };
}