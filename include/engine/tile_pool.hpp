#pragma once

#include <span>
#include <vector>

#include <glm/glm.hpp>

namespace components {
    struct TileProxy {
        std::size_t index = 0;
        std::uint32_t uniqueIdentifier = 0;
    };
}

namespace engine {
    struct TileData {
        std::int64_t order = 0;
    };

    struct alignas(16) TileInstance {
        struct Transform {
            glm::vec2 position;
            glm::vec2 scale;
        } transform;

        struct Appearance {
            struct Texture {
                struct Sample {
                    glm::vec2 position;
                    glm::vec2 extent;
                } sample;

                glm::vec2 offset;
                glm::vec2 repeat;
            } texture;

            glm::vec4 colourFactor;
        } appearance;
    };

    class TilePool {
    public:
        TilePool();

        components::TileProxy insert(const TileInstance& base, std::size_t order);

        TileInstance& getInstance(components::TileProxy proxy);
        TileData& getData(components::TileProxy proxy);

        void remove(components::TileProxy proxy);
        bool contains(components::TileProxy proxy) const;

        void clear();
        void sortByDepth();

        std::span<TileInstance> instances() {
            return instances_;
        }

        std::span<const TileInstance> instances() const {
            return instances_;
        }

        std::span<TileData> data() {
            return data_;
        }

        std::span<const TileData> data() const {
            return data_;
        }

        std::vector<std::size_t>& getProxyGroup(std::size_t index);

        constexpr static std::size_t DeadIndex = std::numeric_limits<std::size_t>::max();

    private:
        std::vector<std::vector<std::size_t>> groupTable_;
        std::vector<std::size_t> table_;
        std::vector<TileInstance> instances_;
        std::vector<TileData> data_;

        static std::uint32_t maxIdentifier_;
        std::uint32_t identifier_;
    };
}