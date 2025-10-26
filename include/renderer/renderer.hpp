#pragma once

#include <app/window.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/command_pool.hpp>
#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/framebuffer.hpp>
#include <renderer/image.hpp>
#include <renderer/image_view.hpp>
#include <renderer/instance.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/render_pass.hpp>
#include <renderer/sampler.hpp>
#include <renderer/semaphore.hpp>
#include <renderer/shader_module.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

namespace renderer {
    class Renderer {
    public:
        Renderer(app::Window& window);
        ~Renderer();

        void acquireImage(const std::vector<Fence>& fences);
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

        auto& getCurrentDepthImage() {
            return depthImages_[imageCounter_.index];
        }

        auto& getCurrentDepthImageView() {
            return depthImageViews_[imageCounter_.index];
        }

        auto& getCurrentSwapchainImage() {
            return Swapchain::getImages(swapchain_)[imageCounter_.index];
        }

        auto& getCurrentSwapchainImageView() {
            return Swapchain::getImageViews(swapchain_)[imageCounter_.index];
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

        renderer::Instance instance_;
        renderer::Surface surface_;
        renderer::Device device_;
        renderer::Swapchain swapchain_;
        renderer::CommandPool commandPool_;
        renderer::RenderPass renderPass_;

        renderer::Queue graphicsQueue_;
        renderer::Queue transferQueue_;
        renderer::Queue presentQueue_;

        std::vector<renderer::Fence> inFlightFences_;
        std::vector<renderer::Semaphore> acquireSemaphores_;
        std::vector<renderer::Semaphore> presentSemaphores_;
        std::vector<renderer::Framebuffer> framebuffers_;
        std::vector<renderer::CommandBuffer> commandBuffers_;
        std::vector<renderer::Image> depthImages_;
        std::vector<renderer::ImageView> depthImageViews_;

        Counter imageCounter_;
        Counter frameCounter_;

        bool resized_ = false;
        bool awaitRestore_ = false;
    };
}