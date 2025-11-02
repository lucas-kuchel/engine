#include <engine/tile_pool.hpp>

#include <algorithm>

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
    return data_[proxy.index];
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
    const std::size_t n = instances_.size();

    if (n <= 1) {
        return;
    }

    struct SortEntry {
        std::size_t proxyIndex;
        std::size_t instanceIndex;
        std::int64_t order;
    };

    std::vector<SortEntry> entries;
    entries.reserve(n);

    for (std::size_t proxyIndex = 0; proxyIndex < table_.size(); ++proxyIndex) {
        std::size_t instIdx = table_[proxyIndex];

        if (instIdx == DeadIndex) {
            continue;
        }

        entries.push_back({proxyIndex, instIdx, data_[proxyIndex].order});
    }

    std::ranges::sort(entries, [](const SortEntry& a, const SortEntry& b) {
        return a.order > b.order;
    });

    std::vector<TileInstance> newInstances;

    newInstances.reserve(n);

    std::vector<std::size_t> newTable(table_.size(), DeadIndex);

    for (std::size_t newPos = 0; newPos < entries.size(); ++newPos) {
        auto& entry = entries[newPos];

        newInstances.push_back(std::move(instances_[entry.instanceIndex]));
        newTable[entry.proxyIndex] = newPos;
    }

    instances_ = std::move(newInstances);
    table_ = std::move(newTable);
}

std::vector<std::size_t>& engine::TilePool::getProxyGroup(std::size_t index) {
    if (groupTable_.size() <= index) {
        groupTable_.resize(index + 1);
    }

    return groupTable_[index];
}