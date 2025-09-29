#pragma once

#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

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
        WindowCreateInfo createWindow();

        renderer::InstanceCreateInfo createInstance();
        renderer::SurfaceCreateInfo createSurface();
        renderer::DeviceCreateInfo createDevice();
        renderer::SwapchainCreateInfo createSwapchain();
        renderer::CommandPool createCommandPool(renderer::Queue& queue);
        renderer::RenderPass createRenderPass();

        Context context_;
        Window window_;

        renderer::Instance instance_;
        renderer::Surface surface_;
        renderer::Device device_;

        std::span<renderer::Queue> deviceQueues_;

        data::NullableReference<renderer::Queue> renderQueue_;
        data::NullableReference<renderer::Queue> presentQueue_;

        renderer::Swapchain swapchain_;
        renderer::CommandPool renderPool_;
        renderer::RenderPass renderPass_;

        std::uint32_t inFlightFrameCount_;
        std::uint32_t inFlightFrameIndex_;

        std::vector<renderer::Semaphore> availableSemaphores_;
        std::vector<renderer::Semaphore> finishedSemaphores_;
        std::vector<renderer::Fence> inFlightFences_;
        std::vector<renderer::CommandBuffer> renderCommandBuffers_;
        std::vector<renderer::Framebuffer> renderFramebuffers_;

        bool running_ = true;
    };
}