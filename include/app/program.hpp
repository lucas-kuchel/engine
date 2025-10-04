#pragma once

#include <data/unique.hpp>

#include <game/instance.hpp>

#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

namespace app {
    struct SettingsConfig {
        data::Extent2D<std::uint32_t> displaySize;

        WindowVisibility displayMode;

        bool vsync;
        bool resizable;

        std::uint32_t imageCount;
        std::uint32_t renderAheadLimit;
    };

    class Program {
    public:
        Program();
        ~Program();

        void run();

        SettingsConfig loadSettings();
        void applySettings(const SettingsConfig& config);

        renderer::Device& device();
        renderer::Swapchain& swapchain();
        renderer::RenderPass& renderPass();
        renderer::CommandPool& transferCommandPool();
        renderer::Queue& transferQueue();
        renderer::Queue& graphicsQueue();

        renderer::CommandBuffer& currentCommandBuffer();
        renderer::Framebuffer& currentFramebuffer();
        renderer::Semaphore& currentAcquireSemaphore();
        renderer::Semaphore& currentPresentSemaphore();
        renderer::Fence& currentFence();

    private:
        void manageBindings(const WindowKeyPressedEventInfo& pressEvent);

        void manageEvents(bool& running);
        void acquireImage(bool& resized);
        void presentImage();

        data::Unique<Context> context_;
        data::Unique<Window> window_;

        data::Unique<renderer::Instance> instance_;
        data::Unique<renderer::Surface> surface_;
        data::Unique<renderer::Device> device_;
        data::Unique<renderer::Swapchain> swapchain_;
        data::Unique<renderer::RenderPass> renderPass_;
        data::Unique<renderer::CommandPool> graphicsCommandPool_;
        data::Unique<renderer::CommandPool> transferCommandPool_;

        data::NullableRef<renderer::Queue> graphicsQueue_;
        data::NullableRef<renderer::Queue> transferQueue_;
        data::NullableRef<renderer::Queue> presentQueue_;

        std::vector<renderer::Fence> inFlightFences_;
        std::vector<renderer::Semaphore> acquireSemaphores_;
        std::vector<renderer::Semaphore> presentSemaphores_;
        std::vector<renderer::Framebuffer> framebuffers_;
        std::vector<renderer::CommandBuffer> commandBuffers_;

        data::Unique<game::Instance> gameInstance_;

        std::uint32_t imageCount_;
        std::uint32_t imageIndex_;
        std::uint32_t frameCount_;
        std::uint32_t frameIndex_;
        std::uint32_t swapchainRecreateImageCount_;
        std::uint32_t swapchainRecreateFrameCount_;

        bool swapchainRecreateSynchronise_;
        bool explicitSwapchainRecreate_ = false;
    };
}