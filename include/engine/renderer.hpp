#pragma once

#include <vulkanite/window/window.hpp>

#include <vulkanite/renderer/renderer.hpp>

namespace engine {
    class Renderer {
    public:
        ~Renderer();

        void create(vulkanite::window::Window& window);
        void acquireImage(const std::vector<vulkanite::renderer::Fence>& fences);
        void presentImage();

        auto& getDevice() {
            return device_;
        }

        auto& getSwapchain() {
            return swapchain_;
        }

        auto& getSurface() {
            return surface_;
        }

        auto& getInstance() {
            return instance_;
        }

        auto getCommandPool() {
            return commandPool_;
        }

        auto& getRenderPass() {
            return renderPass_;
        }

        auto& getGraphicsQueue() {
            return graphicsQueue_;
        }

        auto& getTransferQueue() {
            return transferQueue_;
        }

        auto& getPresentQueue() {
            return presentQueue_;
        }

        auto& getCurrentCommandBuffer() {
            return commandBuffers_[frameCounter_.index];
        }

        auto& getCurrentInFlightFence() {
            return inFlightFences_[frameCounter_.index];
        }

        auto& getCurrentAcquireSemaphore() {
            return acquireSemaphores_[frameCounter_.index];
        }

        auto& getCurrentFramebuffer() {
            return framebuffers_[imageCounter_.index];
        }

        auto& getCurrentPresentSemaphore() {
            return presentSemaphores_[imageCounter_.index];
        }

        auto& getCurrentSwapchainImage() {
            return swapchain_.getImages()[imageCounter_.index];
        }

        auto& getCurrentSwapchainImageView() {
            return swapchain_.getImageViews()[imageCounter_.index];
        }

        auto wasResized() const {
            return resized_;
        }

        void disableAwaitRestore() {
            awaitRestore_ = false;
        }

        auto mustAwaitRestore() const {
            return awaitRestore_;
        }

        auto getFrameCounter() const {
            return frameCounter_;
        }

        auto getImageCounter() const {
            return imageCounter_;
        }

    private:
        struct Counter {
            std::uint32_t count = 0;
            std::uint32_t index = 0;
        };

        vulkanite::renderer::Instance instance_;
        vulkanite::renderer::Surface surface_;
        vulkanite::renderer::Device device_;
        vulkanite::renderer::Swapchain swapchain_;
        vulkanite::renderer::CommandPool commandPool_;
        vulkanite::renderer::RenderPass renderPass_;
        vulkanite::renderer::Queue graphicsQueue_;
        vulkanite::renderer::Queue transferQueue_;
        vulkanite::renderer::Queue presentQueue_;

        std::vector<vulkanite::renderer::Fence> inFlightFences_;
        std::vector<vulkanite::renderer::Semaphore> acquireSemaphores_;
        std::vector<vulkanite::renderer::Semaphore> presentSemaphores_;
        std::vector<vulkanite::renderer::Framebuffer> framebuffers_;
        std::vector<vulkanite::renderer::CommandBuffer> commandBuffers_;

        Counter imageCounter_;
        Counter frameCounter_;

        bool resized_ = false;
        bool awaitRestore_ = false;
    };
}