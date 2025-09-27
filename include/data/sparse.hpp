#pragma once

#include <limits>
#include <stdexcept>
#include <vector>

namespace data {
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
                return dense_[sparse_[index]];
            }

            sparse_[index] = dense_.size();

            dense_.push_back(element);
            reverseMapping_.push_back(index);

            return dense_[sparse_[index]];
        }

        inline void remove(std::size_t index) {
            if (index >= sparse_.size() || sparse_[index] == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("Error calling data::SparseSet::remove(): Index out of range");
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

                dense_[denseIndex] = std::move(dense_[lastIndex]);
                reverseMapping_[denseIndex] = lastEntity;
                sparse_[lastEntity] = denseIndex;

                sparse_[index] = std::numeric_limits<std::size_t>::max();

                dense_.pop_back();
                reverseMapping_.pop_back();
            }
        }

        [[nodiscard]] inline T& get(std::size_t index) {
            if (index >= sparse_.size() || sparse_[index] == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("Error calling data::SparseSet::get(): Index out of range");
            }

            return dense_[sparse_[index]];
        }

        [[nodiscard]] inline const T& get(std::size_t index) const {
            if (index >= sparse_.size() || sparse_[index] == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("Error calling data::SparseSet::get(): Index out of range");
            }

            return dense_[sparse_[index]];
        }

        [[nodiscard]] inline bool contains(std::size_t index) const {
            return index < sparse_.size() && sparse_[index] != std::numeric_limits<std::size_t>::max();
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

        [[nodiscard]] inline std::vector<T>& getdense_() {
            return dense_;
        }

        [[nodiscard]] inline const std::vector<T>& getdense_() const {
            return dense_;
        }

        [[nodiscard]] inline std::vector<std::size_t>& getsparse_() {
            return sparse_;
        }

        [[nodiscard]] inline const std::vector<std::size_t>& getsparse_() const {
            return sparse_;
        }

        [[nodiscard]] inline std::size_t size() const {
            return dense_.size();
        }

        [[nodiscard]] inline bool empty() const {
            return dense_.empty();
        }

        inline void clear() {
            dense_.clear();
            sparse_.clear();
            reverseMapping_.clear();
        }

        inline void shrinkToFit() {
            dense_.shrink_to_fit();
            sparse_.shrink_to_fit();
            reverseMapping_.shrink_to_fit();
        }

    private:
        std::vector<T> dense_;
        std::vector<std::size_t> sparse_;
        std::vector<std::size_t> reverseMapping_;
    };
}