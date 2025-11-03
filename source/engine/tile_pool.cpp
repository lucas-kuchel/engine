#include <engine/tile_pool.hpp>

#include <algorithm>
#include <numeric>

std::uint32_t engine::TilePool::maxIdentifier_ = 0;

engine::TilePool::TilePool()
    : identifier_(maxIdentifier_) {
    maxIdentifier_++;
}

components::TileProxy engine::TilePool::insert(const TileInstance& base, std::int64_t order) {
    std::size_t proxyIndex;

    if (!freed_.empty()) {
        proxyIndex = freed_.back();
        freed_.pop_back();
    }
    else {
        proxyIndex = table_.size();
        table_.push_back(DeadIndex);
    }

    const std::size_t denseIndex = instances_.size();

    instances_.emplace_back(base);
    data_.emplace_back(order);

    if (reverse_.size() <= denseIndex)
        reverse_.resize(denseIndex + 1);

    table_[proxyIndex] = denseIndex;
    reverse_[denseIndex] = proxyIndex;

    return {
        .index = proxyIndex,
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
    if (!contains(proxy)) {
        return;
    }

    const std::size_t sparseIndex = proxy.index;
    const std::size_t denseIndex = table_[sparseIndex];
    const std::size_t lastIndex = instances_.size() - 1;

    if (denseIndex != lastIndex) {
        instances_[denseIndex] = std::move(instances_.back());
        data_[denseIndex] = std::move(data_.back());

        const std::size_t movedProxy = reverse_[lastIndex];

        table_[movedProxy] = denseIndex;
        reverse_[denseIndex] = movedProxy;
    }

    instances_.pop_back();
    data_.pop_back();
    reverse_.pop_back();

    table_[sparseIndex] = DeadIndex;
    freed_.push_back(sparseIndex);
}

bool engine::TilePool::contains(components::TileProxy proxy) const {
    return proxy.uniqueIdentifier == identifier_ && proxy.index < table_.size() && table_[proxy.index] != DeadIndex;
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

    std::vector<std::size_t> order(n);

    std::iota(order.begin(), order.end(), 0);

    std::ranges::sort(order, [&](std::size_t a, std::size_t b) {
        return data_[a].order > data_[b].order;
    });

    std::vector<TileInstance> newInstances(n);
    std::vector<TileData> newData(n);
    std::vector<std::size_t> newReverse(n);

    for (std::size_t i = 0; i < n; ++i) {
        newInstances[i] = std::move(instances_[order[i]]);
        newData[i] = std::move(data_[order[i]]);

        const std::size_t sparseIndex = reverse_[order[i]];

        newReverse[i] = sparseIndex;
        table_[sparseIndex] = i;
    }

    instances_ = std::move(newInstances);
    data_ = std::move(newData);
    reverse_ = std::move(newReverse);
}

std::vector<std::size_t>& engine::TilePool::getProxyGroup(std::size_t index) {
    if (groupTable_.size() <= index) {
        groupTable_.resize(index + 1);
    }

    return groupTable_[index];
}