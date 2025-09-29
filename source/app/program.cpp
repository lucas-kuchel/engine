#include <app/program.hpp>

namespace app {
    Program::Program()
        : window_(createWindow()), instance_(createInstance()), surface_(createSurface()), device_(createDevice()),     // preliminaries
          deviceQueues_(device_.getQueues()), renderQueue_(deviceQueues_[0]), presentQueue_(deviceQueues_[1]),          // queues
          swapchain_(createSwapchain()), renderPool_(createCommandPool(renderQueue_)), renderPass_(createRenderPass()), // postliminaries
          inFlightFrameCount_(0), inFlightFrameIndex_(0) {
    }

    void Program::start() {
        std::uint32_t frameCount = swapchain_.getFrameCount();
        auto swapchainImageViews = swapchain_.getImageViews();
        renderFramebuffers_.reserve(frameCount);

        for (std::size_t i = 0; i < frameCount; i++) {
            auto& imageView = swapchainImageViews[i];

            renderer::FramebufferCreateInfo framebufferCreateInfo = {
                .device = device_,
                .renderPass = renderPass_,
                .imageViews = {imageView},
            };

            renderFramebuffers_.emplace_back(framebufferCreateInfo);
        }

        inFlightFrameCount_ = std::min(frameCount, 3u);

        renderer::CommandBufferCreateInfo renderBufferCreateInfo = {
            .level = renderer::CommandBufferLevel::PRIMARY,
            .count = inFlightFrameCount_,
        };

        renderCommandBuffers_ = renderPool_.allocateCommandBuffers(renderBufferCreateInfo);

        renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
            .device = device_,
        };

        renderer::FenceCreateInfo fenceCreateInfo = {
            .device = device_,
        };

        availableSemaphores_.reserve(inFlightFrameCount_);
        finishedSemaphores_.reserve(inFlightFrameCount_);
        inFlightFences_.reserve(inFlightFrameCount_);

        for (std::size_t i = 0; i < inFlightFrameCount_; i++) {
            availableSemaphores_.emplace_back(semaphoreCreateInfo);
            finishedSemaphores_.emplace_back(semaphoreCreateInfo);
            inFlightFences_.emplace_back(fenceCreateInfo);
        }
    }

    void Program::update() {
        context_.pollEvents();

        std::queue<WindowEvent>& windowEvents = window_.queryEvents();

        while (!windowEvents.empty()) {
            auto& event = windowEvents.front();

            switch (event.type) {
                case WindowEventType::CLOSED:
                    running_ = false;

                default:
                    break;
            }

            windowEvents.pop();
        }

        renderer::Semaphore& availableSemaphore = availableSemaphores_[inFlightFrameIndex_];
        renderer::Semaphore& finishedSemaphore = finishedSemaphores_[inFlightFrameIndex_];
        renderer::Fence& inFlightFence = inFlightFences_[inFlightFrameIndex_];

        inFlightFence.await();
        inFlightFence.reset();

        renderer::CommandBuffer& commandBuffer = renderCommandBuffers_[inFlightFrameIndex_];

        swapchain_.acquireNextImage(availableSemaphore);

        std::uint32_t frameIndex = swapchain_.getFrameIndex();

        renderer::Framebuffer& framebuffer = renderFramebuffers_[frameIndex];

        renderer::Scissor renderPassScissor = {
            .offset = {0, 0},
            .extent = window_.getExtent(),
        };

        renderer::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass_,
            .framebuffer = framebuffer,
            .renderArea = renderPassScissor,
            .clearValues = {
                data::ColourRGBA{0.8, 0.4, 0.2, 1.0},
            },
            .depthClearValue = {},
            .stencilClearValue = {},
        };

        auto commandCapture = commandBuffer.beginCapture();
        auto renderCapture = commandCapture.beginRenderPass(renderPassBeginInfo);

        renderCapture.end();
        commandCapture.end();

        renderQueue_->submit({commandBuffer}, {availableSemaphore}, {finishedSemaphore}, inFlightFence);

        swapchain_.presentNextImage(finishedSemaphore);

        inFlightFrameIndex_ = (inFlightFrameIndex_ + 1) % inFlightFrameCount_;
    }

    void Program::close() {
        renderer::Fence& inFlightFence = inFlightFences_[inFlightFrameIndex_];

        inFlightFence.await();
        inFlightFence.reset();
    }

    bool Program::shouldUpdate() {
        return running_;
    }

    WindowCreateInfo Program::createWindow() {
        return WindowCreateInfo{
            .context = context_,
            .extent = {1280, 720},
            .title = "Engine",
            .visibility = WindowVisibility::WINDOWED,
            .resizable = true,
        };
    }

    renderer::InstanceCreateInfo Program::createInstance() {
        return renderer::InstanceCreateInfo{
            .applicationVersion = {0, 0, 1},
            .engineVersion = {0, 0, 1},
            .applicationName = window_.getTitle(),
            .engineName = "engine",
        };
    }

    renderer::SurfaceCreateInfo Program::createSurface() {
        return renderer::SurfaceCreateInfo{
            .instance = instance_,
            .window = window_,
        };
    }

    renderer::DeviceCreateInfo Program::createDevice() {
        return renderer::DeviceCreateInfo{
            .instance = instance_,
            .queues = {
                renderer::QueueCreateInfo{
                    .type = renderer::QueueType::RENDER,
                    .surface = nullptr,
                },
                renderer::QueueCreateInfo{
                    .type = renderer::QueueType::PRESENT,
                    .surface = surface_,
                },
            },
        };
    }

    renderer::SwapchainCreateInfo Program::createSwapchain() {
        return renderer::SwapchainCreateInfo{
            .instance = instance_,
            .surface = surface_,
            .device = device_,
            .presentQueue = presentQueue_,
            .imageCount = 3,
            .synchronise = true,
        };
    }

    renderer::CommandPool Program::createCommandPool(renderer::Queue& queue) {
        return renderer::CommandPoolCreateInfo{
            .device = device_,
            .queue = queue,
        };
    }

    renderer::RenderPass Program::createRenderPass() {
        return renderer::RenderPassCreateInfo{
            .device = device_,
            .colourAttachments = {
                renderer::FrameAttachmentInfo{
                    .format = swapchain_.getImageFormat(),
                    .loadOperation = renderer::LoadOperation::CLEAR,
                    .storeOperation = renderer::StoreOperation::STORE,
                    .sampleCount = 1,
                },
            },
            .depthAttachment = {},
            .stencilAttachment = {},
        };
    }
}