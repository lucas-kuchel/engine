#pragma once

#include <bitset>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace data {
    template <typename T, std::uint32_t N>
    requires(N > 0)
    class BitsetTree {
    public:
        static constexpr std::uint32_t BitsetSize = N;
        static constexpr std::uint32_t BlockSize = 256;

        static constexpr std::uint32_t RoundedSize = (BitsetSize + 7) / 8 * 8;
        static constexpr std::uint32_t LevelCount = RoundedSize / 8;

        BitsetTree() {
            root_ = pool_.allocate();
            *root_ = {};
        }

        ~BitsetTree() = default;

        BitsetTree(const BitsetTree& other)
            : contiguous_(other.contiguous_) {
            root_ = cloneSubtree(other.root_, pool_);
        }

        BitsetTree(BitsetTree&& other) noexcept
            : root_(other.root_), pool_(std::move(other.pool_)), contiguous_(std::move(other.contiguous_)) {
            other.root_ = nullptr;
        }

        BitsetTree& operator=(const BitsetTree& other) {
            if (this == &other) {
                return *this;
            }

            BitsetTree temp(other);

            std::swap(root_, temp.root_);
            std::swap(pool_, temp.pool_);
            std::swap(contiguous_, temp.contiguous_);

            return *this;
        }

        BitsetTree& operator=(BitsetTree&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            clear(root_);

            root_ = other.root_;
            pool_ = std::move(other.pool_);
            contiguous_ = std::move(other.contiguous_);

            other.root_ = nullptr;

            return *this;
        }

        inline void remove(const std::bitset<BitsetSize>& bitset) {
            removeRecursive(root_, bitset, 0);
        }

        [[nodiscard]] T& insert(const std::bitset<BitsetSize>& bitset) {
            Node* current = root_;

            for (std::uint32_t level = 0; level < LevelCount; level++) {
                std::uint8_t key = getByte(bitset, level);

                Node*& next = current->children[key];

                if (!next) {
                    next = pool_.allocate();
                    *next = {};
                }

                current = next;
            }

            if (!current->entryIndex.has_value()) {
                current->entryIndex = static_cast<std::uint32_t>(contiguous_.size());
                contiguous_.emplace_back(bitset, T{});
            }

            return contiguous_[current->entryIndex.value()].second;
        }

        [[nodiscard]] T& get(const std::bitset<BitsetSize>& bitset) {
            Node* current = root_;

            for (std::uint32_t level = 0; level < LevelCount; level++) {
                std::uint8_t key = getByte(bitset, level);
                Node* next = current->children[key];

                if (!next) {
                    throw std::runtime_error("Call failed: data::BitsetTree::get(): Element does not exist");
                }

                current = next;
            }

            if (!current->entryIndex.has_value()) {
                throw std::runtime_error("Call failed: data::BitsetTree::get(): Element does not exist");
            }

            return contiguous_[current->entryIndex.value()].second;
        }

        [[nodiscard]] const T& get(const std::bitset<BitsetSize>& bitset) const {
            Node* current = root_;

            for (std::uint32_t level = 0; level < LevelCount; level++) {
                std::uint8_t key = getByte(bitset, level);
                Node* next = current->children[key];

                if (!next) {
                    throw std::runtime_error("Call failed: data::BitsetTree::get(): Element does not exist");
                }

                current = next;
            }

            if (!current->entryIndex.has_value()) {
                throw std::runtime_error("Call failed: data::BitsetTree::get(): Element does not exist");
            }

            return contiguous_[current->entryIndex.value()].second;
        }

        [[nodiscard]] auto begin() {
            return contiguous_.begin();
        }

        [[nodiscard]] auto end() {
            return contiguous_.end();
        }

        [[nodiscard]] auto begin() const {
            return contiguous_.begin();
        }

        [[nodiscard]] auto end() const {
            return contiguous_.end();
        }

    private:
        struct Node {
            std::array<Node*, 256> children{};
            std::optional<std::uint32_t> entryIndex;
        };

        class NodePool {
        public:
            NodePool()
                : index(BlockSize) {
            }

            ~NodePool() {
                for (auto* block : blocks) {
                    delete[] block;
                }
            }

            NodePool(const NodePool&) = default;
            NodePool(NodePool&&) noexcept = default;

            NodePool& operator=(const NodePool&) = default;
            NodePool& operator=(NodePool&&) noexcept = default;

            [[nodiscard]] Node* allocate() {
                if (!freeList.empty()) {
                    Node* node = freeList.back();
                    freeList.pop_back();
                    return node;
                }

                if (index >= BlockSize) {
                    allocateBlock();
                }

                return &blocks.back()[index++];
            }

            void deallocate(Node* ptr) {
                if (ptr) {
                    freeList.push_back(ptr);
                }
            }

        private:
            void allocateBlock() {
                blocks.push_back(new Node[BlockSize]);
                index = 0;
            }

            std::vector<Node*> blocks;
            std::vector<Node*> freeList;

            std::uint32_t index;
        };

        void clear(Node*& current) {
            if (!current) {
                return;
            }

            for (auto*& child : current->children) {
                if (child) {
                    clear(child);

                    pool_.deallocate(child);

                    child = nullptr;
                }
            }

            pool_.deallocate(current);
            current = nullptr;
        }

        [[nodiscard]] static std::uint8_t getByte(const std::bitset<BitsetSize>& bitset, std::uint32_t byteIndex) {
            std::uint8_t result = 0;

            for (std::uint32_t i = 0; i < 8; i++) {
                std::uint32_t bitIndex = byteIndex * 8 + i;

                if (bitIndex < BitsetSize && bitset[bitIndex]) {
                    result |= static_cast<std::uint8_t>(1u << i);
                }
            }

            return result;
        }

        [[nodiscard]] Node* findNode(const std::bitset<BitsetSize>& bitset) const {
            Node* current = root_;

            for (std::uint32_t level = 0; level < LevelCount; level++) {
                std::uint8_t key = getByte(bitset, level);
                Node* next = current->children[key];

                if (!next) {
                    return nullptr;
                }

                current = next;
            }
            return current;
        }

        bool removeRecursive(Node*& current, const std::bitset<BitsetSize>& bitset, std::uint32_t level) {
            if (!current) {
                return false;
            }

            if (level == LevelCount) {
                if (current->entryIndex.has_value()) {
                    std::uint32_t removedIndex = current->entryIndex.value();
                    current->entryIndex.reset();

                    if (!contiguous_.empty()) {
                        std::uint32_t lastIndex = static_cast<std::uint32_t>(contiguous_.size() - 1);

                        if (removedIndex != lastIndex) {
                            auto movedPair = std::move(contiguous_[lastIndex]);

                            contiguous_[removedIndex] = std::move(movedPair);

                            Node* movedNode = findNode(contiguous_[removedIndex].first);

                            if (movedNode) {
                                movedNode->entryIndex = removedIndex;
                            }
                        }
                        contiguous_.pop_back();
                    }
                }
            }
            else {
                std::uint8_t key = getByte(bitset, level);
                Node*& next = current->children[key];

                if (removeRecursive(next, bitset, level + 1)) {
                    pool_.deallocate(next);
                    next = nullptr;
                }
            }

            if (current->entryIndex.has_value()) {
                return false;
            }

            for (auto* child : current->children) {
                if (child) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] Node* cloneSubtree(const Node* source, NodePool& pool) {
            if (!source) {
                return nullptr;
            }

            Node* clone = pool.allocate();

            clone->entryIndex = source->entryIndex;

            for (std::uint32_t i = 0; i < 256; i++) {
                if (source->children[i]) {
                    clone->children[i] = cloneSubtree(source->children[i], pool);
                }
            }

            return clone;
        }

        Node* root_;
        NodePool pool_;

        std::vector<std::pair<std::bitset<BitsetSize>, T>> contiguous_;
    };
}