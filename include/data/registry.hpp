#pragma once

#include <data/sparse.hpp>

namespace data {
    // @brief Handle for a resource in a registry
    // @tparam The type stored in the registry
    // @note No instance of the type is stored in this handle
    template <typename T>
    class Handle {
    public:
        // @brief Represents a "dead" or uninitialised handle
        static constexpr Handle<T> Dead = Handle{};

        constexpr Handle()
            : id_(std::numeric_limits<std::size_t>::max()), generation_(std::numeric_limits<std::size_t>::max()) {
        }

        Handle(const Handle&) = default;
        Handle(Handle&&) noexcept = default;

        Handle& operator=(const Handle&) = default;
        Handle& operator=(Handle&&) noexcept = default;

        bool operator==(const Handle<T>& other) const {
            return id_ == other.id_ && generation_ == other.generation_;
        }

        bool operator!=(const Handle<T>& other) const {
            return id_ != other.id_ || generation_ != other.generation_;
        }

        explicit operator bool() const {
            return *this != Dead;
        }

        bool operator<(const Handle<T>& other) const {
            return id_ < other.id_ || (id_ == other.id_ && generation_ < other.generation_);
        }

        bool operator>(const Handle<T>& other) const {
            return id_ > other.id_ || (id_ == other.id_ && generation_ > other.generation_);
        }

        bool operator<=(const Handle<T>& other) const {
            return *this < other || *this == other;
        }

        bool operator>=(const Handle<T>& other) const {
            return *this > other || *this == other;
        }

    private:
        std::size_t id_ = std::numeric_limits<std::size_t>::max();
        std::size_t generation_ = std::numeric_limits<std::size_t>::max();

        template <typename>
        friend class Registry;

        friend struct std::hash<Handle<T>>;
    };

    // @brief Represents a registry of a given type
    // @tparam Any type
    template <typename T>
    class Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&) = default;
        Registry(Registry&&) noexcept = default;

        Registry& operator=(const Registry&) = default;
        Registry& operator=(Registry&&) noexcept = default;

        // @brief Inserts a new element into the registry
        // @param Element to insert
        // @return A handle to the new element
        [[nodiscard]] Handle<T> insert(T&& data) {
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

            data_.insert(handle.id_, std::forward<T>(data));

            return handle;
        }

        // @brief Removes the element corresponding to the handle
        // @param Handle to the element
        // @throws std::runtime_error if handle has no matching entry
        void remove(Handle<T> handle) {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Error calling data::Registry::remove(): Index out of range");
            }

            generations_[handle.id_]++;
            freeList_.push_back(handle.id_);
            data_.remove(handle.id_);
        }

        // @brief Retrieves the element corresponding to the provided handle
        // @param Handle to the element
        // @throws std::runtime_error if the handle has no matching entry
        [[nodiscard]] T& get(Handle<T> handle) {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Error calling data::Registry::get(): Index out of range");
            }

            return data_.get(handle.id_);
        }

        // @brief Retrieves the element corresponding to the provided handle
        // @param Handle to the element
        // @throws std::runtime_error if the handle has no matching entry
        [[nodiscard]] const T& get(Handle<T> handle) const {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Error calling data::Registry::get(): Index out of range");
            }

            return data_.get(handle.id_);
        }

    private:
        SparseSet<T> data_;

        std::vector<std::size_t> freeList_;
        std::vector<std::size_t> generations_;
    };
}

namespace std {
    template <typename T>
    struct hash<data::Handle<T>> {
        std::size_t operator()(const data::Handle<T>& handle) const noexcept {
            std::size_t hash1 = std::hash<std::size_t>{}(handle.id_);
            std::size_t hash2 = std::hash<std::size_t>{}(handle.generation_);

            return hash1 ^ (hash2 << 1);
        }
    };
}
