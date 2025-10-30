#pragma once

#include <components/tile.hpp>

#include <vector>

namespace engine {
    struct TileProxy {
        std::size_t index;
        std::uint32_t uniqueIdentifier;
    };

    class TilePool {
    public:
        TilePool();

        TileProxy insert(const components::TileInstance& base);
        components::TileInstance& get(TileProxy proxy);

        void remove(TileProxy proxy);

        bool contains(TileProxy proxy) const;

        void clear();
        void sortByDepth();

        std::span<components::TileInstance> data();
        std::span<const components::TileInstance> data() const;

        std::vector<TileProxy>& getProxyGroup(std::size_t index);

        constexpr static std::size_t DeadIndex = std::numeric_limits<std::size_t>::max();

    private:
        std::vector<std::size_t> proxyTable_;
        std::vector<std::vector<TileProxy>> groupTable_;
        std::vector<components::TileInstance> instances_;

        static std::uint32_t maxIdentifier_;
        std::uint32_t identifier_;
    };
}