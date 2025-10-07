#include <app/program.hpp>

namespace app {
    Program::Program() {
        game::loadSettings(settings_);

        context_ = data::makeUnique<Context>();

        WindowCreateInfo windowCreateInfo = {
            .context = context_.ref(),
            .extent = {settings_.display.size.x, settings_.display.size.y},
            .title = "Game",
            .visibility = settings_.display.mode,
            .resizable = settings_.display.resizable,
        };

        window_ = data::makeUnique<Window>(windowCreateInfo);

        renderer::InstanceCreateInfo instanceCreateInfo = {
            .applicationName = window_->getTitle(),
            .applicationVersion = data::Version(0, 0, 1),
            .engineName = "engine",
            .engineVersion = data::Version(0, 0, 1),
            .requestDebug = true,
        };

        instance_ = data::makeUnique<renderer::Instance>(instanceCreateInfo);

        renderer::SurfaceCreateInfo surfaceCreateInfo = {
            .instance = instance_.ref(),
            .window = window_.ref(),
        };

        surface_ = data::makeUnique<renderer::Surface>(surfaceCreateInfo);

        renderer::QueueInfo graphicsQueueInfo = {
            .flags = renderer::QueueFlags::GRAPHICS,
            .surface = nullptr,
        };

        renderer::QueueInfo transferQueueInfo = {
            .flags = renderer::QueueFlags::TRANSFER,
            .surface = nullptr,
        };

        renderer::QueueInfo presentQueueInfo = {
            .flags = renderer::QueueFlags::PRESENT,
            .surface = surface_.ref(),
        };

        renderer::DeviceCreateInfo deviceCreateInfo = {
            .instance = instance_.ref(),
            .queues = {
                graphicsQueueInfo,
                transferQueueInfo,
                presentQueueInfo,
            },
        };

        device_ = data::makeUnique<renderer::Device>(deviceCreateInfo);

        std::span<renderer::Queue> queues = device_->queues();

        graphicsQueue_ = queues[0];
        transferQueue_ = queues[1];
        presentQueue_ = queues[2];

        renderer::SwapchainCreateInfo swapchainCreateInfo = {
            .surface = surface_.ref(),
            .device = device_.ref(),
            .presentQueue = presentQueue_.get(),
            .imageCount = settings_.graphics.imageCount,
            .synchronise = settings_.graphics.vsync,
        };

        swapchain_ = data::makeUnique<renderer::Swapchain>(swapchainCreateInfo);

        renderer::CommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
            .device = device_.ref(),
            .queue = graphicsQueue_.get(),
        };

        renderer::CommandPoolCreateInfo transferCommandPoolCreateInfo = {
            .device = device_.ref(),
            .queue = transferQueue_.get(),
        };

        graphicsCommandPool_ = data::makeUnique<renderer::CommandPool>(graphicsCommandPoolCreateInfo);
        transferCommandPool_ = data::makeUnique<renderer::CommandPool>(transferCommandPoolCreateInfo);

        renderer::RenderPassCreateInfo renderPassCreateInfo = {
            .device = device_.ref(),
            .depthStencilAttachments = {},
            .colourAttachments = {
                renderer::ColourAttachmentInfo{
                    .format = swapchain_->format(),
                    .initialLayout = renderer::ImageLayout::UNDEFINED,
                    .finalLayout = renderer::ImageLayout::PRESENT_SOURCE,
                    .operations = {
                        .load = renderer::LoadOperation::CLEAR,
                        .store = renderer::StoreOperation::STORE,
                    }},
            },
            .subpasses = {
                renderer::SubpassInfo{
                    .colourAttachmentInputIndices = {},
                    .colourAttachmentOutputIndices = {0},
                    .depthStencilIndex = {},
                },
            },
            .subpassDependencies = {},
            .sampleCount = 1,
        };

        renderPass_ = data::makeUnique<renderer::RenderPass>(renderPassCreateInfo);

        imageCounter_.count = swapchain_->imageCount();
        imageCounter_.index = 0;

        frameCounter_.count = std::min(imageCounter_.count, settings_.graphics.renderAheadLimit);
        frameCounter_.index = 0;

        renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
            .device = device_.ref(),
        };

        renderer::FenceCreateInfo fenceCreateInfo = {
            .device = device_.ref(),
            .createFlags = renderer::FenceCreateFlags::START_SIGNALED,
        };

        std::span<renderer::ImageView> swapchainImages = swapchain_->images();

        presentSemaphores_.reserve(imageCounter_.count);
        framebuffers_.reserve(imageCounter_.count);

        for (std::size_t i = 0; i < imageCounter_.count; i++) {
            renderer::FramebufferCreateInfo framebufferCreateInfo = {
                .device = device_.ref(),
                .renderPass = renderPass_.ref(),
                .imageViews = {swapchainImages[i]},
            };

            framebuffers_.emplace_back(framebufferCreateInfo);
            presentSemaphores_.emplace_back(semaphoreCreateInfo);
        }

        acquireSemaphores_.reserve(frameCounter_.count);
        inFlightFences_.reserve(frameCounter_.count);

        for (std::size_t i = 0; i < frameCounter_.count; i++) {
            acquireSemaphores_.emplace_back(semaphoreCreateInfo);
            inFlightFences_.emplace_back(fenceCreateInfo);
        }

        renderer::CommandBufferCreateInfo commandBufferCreateInfo = {
            .count = frameCounter_.count,
        };

        commandBuffers_ = graphicsCommandPool_->allocateCommandBuffers(commandBufferCreateInfo);

        run();
    }

    Program::~Program() {
        if (device_) {
            device_->waitIdle();
        }
    }

    void Program::acquireImage(bool& resized) {
        resized = false;

        if (explicitSwapchainRecreate_) {
            explicitSwapchainRecreate_ = false;

            renderer::SwapchainRecreateInfo swapchainRecreateInfo = {
                .imageCount = settings_.graphics.imageCount,
                .synchronise = settings_.graphics.vsync,
            };

            device_->waitIdle();
            swapchain_->recreate(swapchainRecreateInfo);

            resized = true;

            frameCounter_.count = std::min(imageCounter_.count, settings_.graphics.renderAheadLimit);
            frameCounter_.index = std::min(frameCounter_.index, frameCounter_.count - 1);

            acquireSemaphores_.clear();
            inFlightFences_.clear();

            renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
                .device = device_.ref(),
            };

            renderer::FenceCreateInfo fenceCreateInfo = {
                .device = device_.ref(),
                .createFlags = renderer::FenceCreateFlags::START_SIGNALED,
            };

            acquireSemaphores_.reserve(frameCounter_.count);
            inFlightFences_.reserve(frameCounter_.count);

            for (std::size_t i = 0; i < frameCounter_.count; i++) {
                acquireSemaphores_.emplace_back(semaphoreCreateInfo);
                inFlightFences_.emplace_back(fenceCreateInfo);
            }

            graphicsCommandPool_->destroyCommandBuffers(commandBuffers_);

            renderer::CommandBufferCreateInfo commandBufferCreateInfo = {
                .count = frameCounter_.count,
            };

            commandBuffers_ = graphicsCommandPool_->allocateCommandBuffers(commandBufferCreateInfo);
        }

        renderer::Semaphore& acquireSemaphore = acquireSemaphores_[frameCounter_.index];
        renderer::Fence& inFlightFence = inFlightFences_[frameCounter_.index];

        device_->waitForFences({inFlightFence});
        device_->resetFences({inFlightFence});

        while (true) {
            if (swapchain_->shouldRecreate()) {
                renderer::SwapchainRecreateInfo swapchainRecreateInfo = {
                    .imageCount = swapchain_->imageCount(),
                    .synchronise = swapchain_->synchronised(),
                };

                device_->waitIdle();
                swapchain_->recreate(swapchainRecreateInfo);

                resized = true;
            }

            imageCounter_.index = swapchain_->acquireNextImage(acquireSemaphore);

            if (!swapchain_->shouldRecreate()) {
                break;
            }
        }

        if (resized) {
            framebuffers_.clear();
            presentSemaphores_.clear();

            imageCounter_.count = swapchain_->imageCount();

            renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
                .device = device_.ref(),
            };

            std::span<renderer::ImageView> swapchainImages = swapchain_->images();

            for (std::size_t i = 0; i < imageCounter_.count; i++) {
                renderer::FramebufferCreateInfo framebufferCreateInfo = {
                    .device = device_.ref(),
                    .renderPass = renderPass_.ref(),
                    .imageViews = {swapchainImages[i]},
                };

                framebuffers_.emplace_back(framebufferCreateInfo);
                presentSemaphores_.emplace_back(semaphoreCreateInfo);
            }
        }
    }

    void Program::presentImage() {
        renderer::Semaphore& presentSemaphore = presentSemaphores_[imageCounter_.index];

        swapchain_->presentNextImage(presentSemaphore);

        frameCounter_.index = (frameCounter_.index + 1) % frameCounter_.count;
    }

    void Program::manageEvents(bool& running) {
        context_->pollEvents();

        while (window_->hasEvents()) {
            WindowEvent event = window_->getNextEvent();

            switch (event.type) {
                case WindowEventType::CLOSED:
                    running = false;
                    break;

                case WindowEventType::KEY_PRESSED:
                    game::updateCharacterVelocity(playerCharacter_, playerController_, event.info.keyPress);
                    break;

                case WindowEventType::KEY_RELEASED:
                    game::updateCharacterVelocity(playerCharacter_, playerController_, event.info.keyRelease);
                    break;

                default:
                    break;
            }
        }
    }

    void Program::run() {
        start();

        bool running = true;
        bool resized = false;

        while (running) {
            manageEvents(running);
            acquireImage(resized);

            update();
            render();

            presentImage();
        }

        close();
    }
}