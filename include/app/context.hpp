#pragma once

#include <memory>

namespace app {
    // @brief Current platform backend
    enum class ContextBackend {
        WAYLAND,
        X11,
        WIN32,
        COCOA,
    };

    // @brief Represents the interaction layer between the platform and window
    // @note Not safe to copy
    class Context {
    public:
        Context();
        ~Context();

        Context(const Context&) = delete;
        Context(Context&&) noexcept = default;

        Context& operator=(const Context&) = delete;
        Context& operator=(Context&&) noexcept = default;

        // @brief Polls events and provides them to all related windows
        // @throws std::runtime_error if the context is invalid
        void pollEvents();

        // @brief Waits for events and provides them to all related windows
        // @throws std::runtime_error if the context is invalid
        void awaitEvents();

        // @brief Waits for events with a timeout and provides them to all related windows
        // @param Timeout for the wait period
        // @throws std::runtime_error if the context is invalid
        void awaitEventsTimeout(double timeout);

        // @brief Provides the current backend
        // @throws std::runtime_error if the context is invalid
        ContextBackend queryBackend() const;

        // @brief Enumerates all extensions required for a Vulkan instance
        // @param Pointer to extension count
        // @return C-style array of extension names
        // @throws std::runtime_error if the context is invalid
        const char** enumerateRequiredInstanceExtensions(std::uint32_t* count);

    private:
        class Implementation;

        std::unique_ptr<Implementation> implementation_;
    };
}