#pragma once

#include <data/sparse.hpp>

namespace data {
    template <typename T>
    class Handle {
    private:
        std::size_t id_ = 0;
        std::size_t generation_ = 0;

        template <typename>
        friend class UniqueRegistry;
    };

    template <typename T>
    class UniqueRegistry {
    public:
        UniqueRegistry() = default;
        ~UniqueRegistry() = default;

        UniqueRegistry(const UniqueRegistry&) = default;
        UniqueRegistry(UniqueRegistry&&) noexcept = default;

        UniqueRegistry& operator=(const UniqueRegistry&) = default;
        UniqueRegistry& operator=(UniqueRegistry&&) noexcept = default;

        Handle<T> insert(const T& data) {
            Handle<T> handle;

            if (!freeList_.empty()) {
                handle.id_ = freeList_.back();
                handle.generation_ = generations_[handle.id_];
                freeList_.pop_back();
            }
            else {
                handle.id_ = generations_.size();
                generations_.push_back(0);
                handle.generation_ = 0;
            }

            data_.insert(handle.id_, data);

            return handle;
        }

        void remove(Handle<T> handle) {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Error calling data::UniqueRegistry::remove(): Index out of range");
            }

            generations_[handle.id_]++;
            freeList_.push_back(handle.id_);
            data_.remove(handle.id_);
        }

        T& get(Handle<T> handle) {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Error calling data::UniqueRegistry::get(): Index out of range");
            }

            return data_.get(handle.id_);
        }

        const T& get(Handle<T> handle) const {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Error calling data::UniqueRegistry::get(): Index out of range");
            }

            return data_.get(handle.id_);
        }

    private:
        SparseSet<T> data_;

        std::vector<std::size_t> freeList_;
        std::vector<std::size_t> generations_;
    };
}