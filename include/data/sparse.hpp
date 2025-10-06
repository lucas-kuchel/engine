#pragma once

#include <limits>
#include <stdexcept>
#include <vector>

namespace data {
    // @brief Represents a sparse set of elements stored contiguously in memory
    template <typename T>
    class SparseSet {
    public:
        inline SparseSet() = default;
        inline ~SparseSet() = default;

        inline SparseSet(const SparseSet&) = default;
        inline SparseSet& operator=(const SparseSet&) = default;

        inline SparseSet(SparseSet&&) noexcept = default;
        inline SparseSet& operator=(SparseSet&&) noexcept = default;

        inline T& insert(std::size_t index, const T& element) {
            if (index >= sparse_.size()) {
                sparse_.resize(index + 1, std::numeric_limits<std::size_t>::max());
            }

            if (sparse_[index] != std::numeric_limits<std::size_t>::max()) {
                dense_[sparse_[index]] = std::forward<T>(element);

                return dense_[sparse_[index]];
            }

            sparse_[index] = dense_.size();

            dense_.emplace_back(element);
            reverseMapping_.push_back(index);

            return dense_[sparse_[index]];
        }

        // @brief Inserts an element at the provided index
        // @param The index to insert to
        // @param The element to insert
        // @return The inserted element
        // @note If the index is already occupied it will overwrite the value
        inline T& insert(std::size_t index, T&& element) {
            if (index >= sparse_.size()) {
                sparse_.resize(index + 1, std::numeric_limits<std::size_t>::max());
            }

            if (sparse_[index] != std::numeric_limits<std::size_t>::max()) {
                dense_[sparse_[index]] = std::forward<T>(element);

                return dense_[sparse_[index]];
            }

            sparse_[index] = dense_.size();

            dense_.emplace_back(std::forward<T>(element));
            reverseMapping_.push_back(index);

            return dense_[sparse_[index]];
        }

        // @brief Removes the element at the provided index
        // @param The index to remove
        // @throws std::runtime_error if the index has no value
        inline void remove(std::size_t index) {
            if (index >= sparse_.size() || sparse_[index] == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("Call failed: data::SparseSet::remove(): Index out of range");
            }

            std::size_t denseIndex = sparse_[index];
            std::size_t lastIndex = static_cast<std::size_t>(dense_.size() - 1);

            if (denseIndex == lastIndex) {
                sparse_[index] = std::numeric_limits<std::size_t>::max();

                dense_.pop_back();
                reverseMapping_.pop_back();
            }
            else {
                std::size_t lastEntity = reverseMapping_[lastIndex];

                std::swap(dense_[denseIndex], dense_[lastIndex]);
                std::swap(reverseMapping_[denseIndex], reverseMapping_[lastIndex]);

                sparse_[lastEntity] = denseIndex;
                sparse_[index] = std::numeric_limits<std::size_t>::max();

                dense_.pop_back();
                reverseMapping_.pop_back();
            }
        }

        // @brief Retrieves the element at the provided index
        // @param The index to get
        // @return The element corresponding to the index
        // @throws std::runtime_error if the index has no value
        [[nodiscard]] inline T& get(std::size_t index) {
            if (index >= sparse_.size() || sparse_[index] == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("Call failed: data::SparseSet::get(): Index out of range");
            }

            return dense_[sparse_[index]];
        }

        // @brief Retrieves the element at the provided index
        // @param The index to get
        // @return The element corresponding to the index
        // @throws std::runtime_error if the index has no value
        [[nodiscard]] inline const T& get(std::size_t index) const {
            if (index >= sparse_.size() || sparse_[index] == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("Call failed: data::SparseSet::get(): Index out of range");
            }

            return dense_[sparse_[index]];
        }

        // @brief Checks if the index exists in the set
        // @param The index to check
        // @return If the element exists
        [[nodiscard]] inline bool contains(std::size_t index) const {
            return index < sparse_.size() && sparse_[index] != std::numeric_limits<std::size_t>::max();
        }

        // @brief Provides the size of the set
        // @return The size of the set
        [[nodiscard]] inline std::size_t size() const {
            return dense_.size();
        }

        // @brief Checks if the set is empty
        // @return If the set is empty
        [[nodiscard]] inline bool empty() const {
            return dense_.empty();
        }

        // @brief Clears the set
        inline void clear() {
            dense_.clear();
            sparse_.clear();
            reverseMapping_.clear();
        }

        // @brief Deallocates all unused memory
        // @note Not guaranteed to reduce memory usage; dependent on the STL
        inline void shrinkToFit() {
            dense_.shrink_to_fit();
            sparse_.shrink_to_fit();
            reverseMapping_.shrink_to_fit();
        }

        [[nodiscard]] inline auto begin() noexcept {
            return dense_.begin();
        }

        [[nodiscard]] inline auto begin() const noexcept {
            return dense_.begin();
        }

        [[nodiscard]] inline auto end() noexcept {
            return dense_.end();
        }

        [[nodiscard]] inline auto end() const noexcept {
            return dense_.end();
        }

        [[nodiscard]] inline auto cbegin() noexcept {
            return dense_.cbegin();
        }

        [[nodiscard]] inline auto cend() const noexcept {
            return dense_.cend();
        }

        std::vector<T>& dense() {
            return dense_;
        }

        const std::vector<T>& dense() const {
            return dense_;
        }

        std::vector<std::size_t>& sparse() {
            return sparse_;
        }

        const std::vector<std::size_t>& sparse() const {
            return sparse_;
        }

    private:
        std::vector<T> dense_;
        std::vector<std::size_t> sparse_;
        std::vector<std::size_t> reverseMapping_;
    };
}