#pragma once

#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

namespace app {
    // @brief Represents the whole program
    // @note Not safe to move or copy
    class Program {
    public:
        Program();
        ~Program() = default;

        Program(const Program&) = delete;
        Program(Program&&) noexcept = delete;

        Program& operator=(const Program&) = delete;
        Program& operator=(Program&&) noexcept = delete;

        // @brief Starts the program
        void start();

        // @brief Updates the program
        void update();

        // @brief Closes the program
        void close();

        // @brief Signals whether the program should update
        // @return Whether the program should update
        [[nodiscard]] bool shouldUpdate();

    private:
        Context context_;
        Window window_;

        renderer::Instance instance_;
        renderer::Surface surface_;
        renderer::Device device_;
        renderer::Swapchain swapchain_;

        renderer::Queue renderQueue_;
        renderer::Queue presentQueue_;

        bool running_ = true;
    };
}