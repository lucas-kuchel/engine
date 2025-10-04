#pragma once

#include <stdexcept>
#include <vector>

namespace data {
    // @brief Represents a non-owning reference to a resource
    // @note Cannot be null but can be reassigned
    // @note Does not promise a valid reference; dangling references can still occur
    // @note Default bool behaviour is on the underlying type
    // @tparam Any type
    template <typename T>
    class Ref {
    public:
        Ref(T& ref)
            : raw_(&ref) {
        }

        Ref(const Ref&) = default;
        Ref(Ref&&) noexcept = default;

        Ref& operator=(const Ref&) = default;
        Ref& operator=(Ref&&) noexcept = default;

        Ref& operator=(T& other) noexcept {
            raw_ = &other;

            return *this;
        }

        T* operator->() {
            return raw_;
        }

        const T* operator->() const {
            return raw_;
        }

        bool operator==(Ref<T>& other) const {
            return raw_ == other.raw_;
        }

        bool operator==(T& other) const {
            return raw_ == &other;
        }

        bool operator!=(Ref<T>& other) const {
            return raw_ != other.raw_;
        }

        bool operator!=(T& other) const {
            return raw_ != &other;
        }

        T& get() {
            return *raw_;
        }

        const T& get() const {
            return *raw_;
        }

    private:
        T* raw_;

        template <typename>
        friend class NullableRef;
    };

    // @brief Represents a non-owning reference to a resource
    // @note Can be null and reassigned
    // @note Does not promise a valid reference; dangling references can still occur
    // @note Default bool behaviour is on the underlying type
    // @throws std::runtime_error when accessing a null reference
    // @tparam Any type
    template <typename T>
    class NullableRef {
    public:
        NullableRef()
            : raw_(nullptr) {
        }

        NullableRef(T& ref)
            : raw_(&ref) {
        }

        NullableRef(std::nullptr_t null)
            : raw_(null) {
        }

        NullableRef(const NullableRef&) = default;
        NullableRef(NullableRef&) noexcept = default;

        NullableRef& operator=(const NullableRef&) = default;
        NullableRef& operator=(NullableRef&&) noexcept = default;

        NullableRef& operator=(T& other) {
            raw_ = &other;

            return *this;
        }

        NullableRef& operator=(const Ref<T>& ref) noexcept {
            raw_ = ref.raw_;

            return *this;
        }

        NullableRef& operator=(std::nullptr_t other) {
            raw_ = other;

            return *this;
        }

        T* operator->() {
            if (!raw_) {
                throw std::runtime_error("Illegal explicit dereference of data::NullableRef: Value is null");
            }

            return raw_;
        }

        const T* operator->() const {
            if (!raw_) {
                throw std::runtime_error("Illegal explicit dereference of data::NullableRef: Value is null");
            }

            return raw_;
        }

        bool operator==(NullableRef<T>& other) const {
            return raw_ == other.raw_;
        }

        bool operator==(T& other) const {
            return raw_ == &other;
        }

        bool operator==(std::nullptr_t other) const {
            return raw_ == other;
        }

        bool operator!=(NullableRef<T>& other) const {
            return raw_ != other.raw_;
        }

        bool operator!=(T& other) const {
            return raw_ != &other;
        }

        bool operator!=(std::nullptr_t other) const {
            return raw_ != other;
        }

        T& get() {
            if (!raw_) {
                throw std::runtime_error("Illegal explicit dereference of data::NullableRef: Value is null");
            }

            return *raw_;
        }

        const T& get() const {
            if (!raw_) {
                throw std::runtime_error("Illegal explicit dereference of data::NullableRef: Value is null");
            }

            return *raw_;
        }

    private:
        T* raw_;
    };

    template <typename T>
    using ReferenceList = std::vector<data::Ref<T>>;

    template <typename T>
    using NullableRefList = std::vector<data::NullableRef<T>>;
}