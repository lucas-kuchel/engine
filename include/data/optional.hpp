#pragma once

#include <stdexcept>
#include <utility>

namespace data {
    // @brief Represents an owning optional value
    // @note Safe to copy and move
    // @note Default bool behaviour is for if the value exists
    // @throws std::runtime_error on illegal dereference
    template <typename T>
    class Optional {
    public:
        Optional() : exists_(false) {
        }

        Optional(T&& value) : value_(std::forward<T>(value)), exists_(true) {
        }

        Optional(const Optional&) = default;
        Optional(Optional&&) noexcept = default;

        Optional& operator=(const Optional&) = default;
        Optional& operator=(Optional&&) noexcept = default;

        T& get() {
            if (!exists_) {
                throw std::runtime_error("Illegal implicit dereference of data::Optional: Value does not exist");
            }

            return value_;
        }

        const T& get() const {
            if (!exists_) {
                throw std::runtime_error("Illegal implicit dereference of data::Optional: Value does not exist");
            }

            return value_;
        }

        T* operator->() {
            if (!exists_) {
                throw std::runtime_error("Illegal explicit dereference of data::Optional: Value does not exist");
            }

            return &value_;
        }

        const T* operator->() const {
            if (!exists_) {
                throw std::runtime_error("Illegal explicit dereference of data::Optional: Value does not exist");
            }

            return &value_;
        }

        explicit operator bool() const {
            return exists_;
        }

    private:
        T value_;
        bool exists_;
    };
}