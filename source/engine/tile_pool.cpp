#include <engine/tile_pool.hpp>

#include <algorithm>
#include <ranges>

std::uint32_t engine::TilePool::maxIdentifier_ = 0;

engine::TilePool::TilePool()
    : identifier_(maxIdentifier_) {
    maxIdentifier_++;
}

components::TileProxy engine::TilePool::insert(const TileInstance& base, std::size_t order) {
    auto index = instances_.size();

    table_.emplace_back(index);
    instances_.emplace_back(base);
    data_.emplace_back(order);

    return {
        .index = index,
        .uniqueIdentifier = identifier_,
    };
}

engine::TileInstance& engine::TilePool::getInstance(components::TileProxy proxy) {
    return instances_[table_[proxy.index]];
}

engine::TileData& engine::TilePool::getData(components::TileProxy proxy) {
    return data_[table_[proxy.index]];
}

void engine::TilePool::remove(components::TileProxy proxy) {
    auto remove = [&](auto& data) {
        std::swap(data[table_[proxy.index]], data.back());

        data.pop_back();

        table_[proxy.index] = DeadIndex;
    };

    remove(instances_);
    remove(data_);
}

bool engine::TilePool::contains(components::TileProxy proxy) const {
    return proxy.uniqueIdentifier == identifier_ && proxy.index < table_.size();
}

void engine::TilePool::clear() {
    table_.clear();
    instances_.clear();
    data_.clear();
}

void engine::TilePool::sortByDepth() {
    auto condition = [](auto const& lhs, auto const& rhs) {
        auto& [tableL, dataL, instanceL] = lhs;
        auto& [tableR, dataR, instanceR] = rhs;

        return dataL.order < dataR.order;
    };

    auto zipped = std::views::zip(table_, data_, instances_);

    std::ranges::sort(zipped, condition, {});
}

std::vector<std::size_t>& engine::TilePool::getProxyGroup(std::size_t index) {
    if (groupTable_.size() <= index) {
        groupTable_.resize(index + 1);
    }

    return groupTable_[index];
}