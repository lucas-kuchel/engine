#pragma once

#include <stdexcept>

namespace data {
    // @brief Represents a non-owning reference to a resource
    // @note Cannot be null but can be reassigned
    // @note Does not promise a valid reference; dangling references can still occur
    // @note Default bool behaviour is on the underlying type
    // @tparam Any type
    template <typename T>
    class Reference {
    public:
        Reference(T& ref)
            : raw_(&ref) {
        }

        Reference(const Reference&) = default;
        Reference(Reference&&) noexcept = default;

        Reference& operator=(const Reference&) = default;
        Reference& operator=(Reference&&) noexcept = default;

        Reference& operator=(const T& other) noexcept {
            *raw_ = other;

            return *this;
        }

        template <typename U>
        Reference& operator=(U&& other) noexcept {
            *raw_ = std::forward<U>(other);

            return *this;
        }

        operator T&() {
            return *raw_;
        }

        operator const T&() const {
            return *raw_;
        }

        T* operator->() {
            return raw_;
        }

        const T* operator->() const {
            return raw_;
        }

        bool operator==(Reference<T>& other) const {
            return raw_ == other.raw_;
        }

        bool operator==(T& other) const {
            return *raw_ == other.raw_;
        }

        bool operator!=(Reference<T>& other) const {
            return raw_ != other.raw_;
        }

        bool operator!=(T& other) const {
            return *raw_ != other.raw_;
        }

    private:
        T* raw_;

        template <typename>
        friend class NullableReference;
    };

    // @brief Represents a non-owning reference to a resource
    // @note Can be null and reassigned
    // @note Does not promise a valid reference; dangling references can still occur
    // @note Default bool behaviour is on the underlying type
    // @throws std::runtime_error when accessing a null reference
    // @tparam Any type
    template <typename T>
    class NullableReference {
    public:
        NullableReference()
            : raw_(nullptr) {
        }

        NullableReference(T& ref)
            : raw_(&ref) {
        }

        NullableReference(std::nullptr_t null)
            : raw_(null) {
        }

        NullableReference(const NullableReference&) = default;
        NullableReference(NullableReference&) noexcept = default;

        NullableReference& operator=(const NullableReference&) = default;
        NullableReference& operator=(NullableReference&&) noexcept = default;

        NullableReference& operator=(const T& other) {
            if (!raw_) {
                throw std::runtime_error("Illegal implicit dereference of data::NullableReference: Value is null");
            }

            *raw_ = other;

            return *this;
        }

        template <typename U>
        NullableReference& operator=(U&& other) {
            if (!raw_) {
                throw std::runtime_error("Illegal implicit dereference of data::NullableReference: Value is null");
            }

            *raw_ = std::forward<U>(other);

            return *this;
        }

        NullableReference& operator=(const Reference<T>& ref) noexcept {
            raw_ = ref.raw_;

            return *this;
        }

        NullableReference& operator=(std::nullptr_t other) {
            raw_ = other;

            return *this;
        }

        operator T&() {
            if (!raw_) {
                throw std::runtime_error("Illegal implicit dereference of data::NullableReference: Value is null");
            }

            return *raw_;
        }

        operator const T&() const {
            if (!raw_) {
                throw std::runtime_error("Illegal implicit dereference of data::NullableReference: Value is null");
            }

            return *raw_;
        }

        T* operator->() {
            if (!raw_) {
                throw std::runtime_error("Illegal explicit dereference of data::NullableReference: Value is null");
            }

            return raw_;
        }

        const T* operator->() const {
            if (!raw_) {
                throw std::runtime_error("Illegal explicit dereference of data::NullableReference: Value is null");
            }

            return raw_;
        }

        bool operator==(NullableReference<T>& other) const {
            return raw_ == other.raw_;
        }

        bool operator==(T& other) const {
            if (!raw_) {
                throw std::runtime_error("Illegal implicit dereference of data::NullableReference: Value is null");
            }

            return *raw_ == other.raw_;
        }

        bool operator==(std::nullptr_t other) const {
            return raw_ == other;
        }

        bool operator!=(NullableReference<T>& other) const {
            return raw_ != other.raw_;
        }

        bool operator!=(T& other) const {
            if (!raw_) {
                throw std::runtime_error("Illegal implicit dereference of data::NullableReference: Value is null");
            }

            return *raw_ != other.raw_;
        }

        bool operator!=(std::nullptr_t other) const {
            return raw_ != other;
        }

    private:
        T* raw_;
    };
}