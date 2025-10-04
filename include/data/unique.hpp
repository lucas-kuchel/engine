#pragma once

#include <utility>

namespace data {
    template <typename T>
    class Unique {
    public:
        Unique() noexcept : raw_(nullptr) {
        }
        explicit Unique(std::nullptr_t) noexcept : raw_(nullptr) {
        }

        explicit Unique(T* ptr) noexcept : raw_(ptr) {
        }

        Unique(const Unique&) = delete;
        Unique& operator=(const Unique&) = delete;

        Unique(Unique&& other) noexcept : raw_(other.raw_) {
            other.raw_ = nullptr;
        }

        Unique& operator=(Unique&& other) noexcept {
            if (this != &other) {
                delete raw_;
                raw_ = other.raw_;
                other.raw_ = nullptr;
            }
            return *this;
        }

        ~Unique() {
            delete raw_;
        }

        T& ref() {
            return *raw_;
        }
        const T& ref() const {
            return *raw_;
        }

        T* raw() {
            return raw_;
        }
        const T* raw() const {
            return raw_;
        }

        explicit operator bool() const noexcept {
            return raw_ != nullptr;
        }

        T* operator->() noexcept {
            return raw_;
        }
        const T* operator->() const noexcept {
            return raw_;
        }

        T& operator*() noexcept {
            return *raw_;
        }
        const T& operator*() const noexcept {
            return *raw_;
        }

        T* release() noexcept {
            T* tmp = raw_;
            raw_ = nullptr;
            return tmp;
        }

        void reset(T* ptr = nullptr) noexcept {
            if (raw_ != ptr) {
                delete raw_;
                raw_ = ptr;
            }
        }

    private:
        T* raw_;
    };

    template <typename T, typename... Args>
    Unique<T> makeUnique(Args&&... args) {
        return Unique<T>(new T(std::forward<Args>(args)...));
    }
}