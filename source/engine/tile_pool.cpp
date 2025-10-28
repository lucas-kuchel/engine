#include <engine/tile_pool.hpp>

engine::TileProxy engine::TilePool::insert(const components::TileInstance& base) {
    auto index = instances_.size();
    proxyTable_.emplace_back(index);
    instances_.emplace_back(base);

    return {index};
}

components::TileInstance& engine::TilePool::get(TileProxy proxy) {
    return instances_[proxyTable_[proxy.index]];
}

void engine::TilePool::remove(TileProxy proxy) {
    auto index = proxyTable_[proxy.index];
    proxyTable_[proxy.index] = DeadIndex;

    std::swap(instances_[index], instances_.back());

    instances_.pop_back();
}

void engine::TilePool::clear() {
    proxyTable_.clear();
    instances_.clear();
}

void engine::TilePool::sortByDepth() {
    auto sortProxies = [this](const std::size_t& a, const std::size_t& b) {
        return instances_[a].transform.position.z < instances_[b].transform.position.z;
    };

    auto sortInstances = [](const components::TileInstance& a, const components::TileInstance& b) {
        return a.transform.position.z < b.transform.position.z;
    };

    std::sort(proxyTable_.begin(), proxyTable_.end(), sortProxies);
    std::sort(instances_.begin(), instances_.end(), sortInstances);
}

std::span<components::TileInstance> engine::TilePool::data() {
    return instances_;
}

std::span<const components::TileInstance> engine::TilePool::data() const {
    return instances_;
}

std::vector<engine::TileProxy>& engine::TilePool::getProxyGroup(std::size_t index) {
    if (proxyTable_.size() <= index) {
        proxyTable_.resize(index + 1);
    }

    return groupTable_[index];
}