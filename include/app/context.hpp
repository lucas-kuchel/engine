#pragma once

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
        void pollEvents();

        // @brief Waits for events and provides them to all related windows
        // @throws std::runtime_error if the context is invalid
        void awaitEvents();

        // @brief Waits for events with a timeout and provides them to all related windows
        // @param Timeout for the wait period
        void awaitEventsTimeout(double timeout);

        // @brief Provides the current backend
        ContextBackend queryBackend() const;
    };
}