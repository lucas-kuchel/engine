#include <app/program.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

namespace app {
    Program::Program() {
        SettingsConfig settingsConfig = loadSettings();

        gameInstance_ = data::makeUnique<game::Instance>(*this);

        context_ = data::makeUnique<Context>();

        WindowCreateInfo windowCreateInfo = {
            .context = context_.ref(),
            .extent = settingsConfig.displaySize,
            .title = "Game",
            .visibility = settingsConfig.displayMode,
            .resizable = settingsConfig.resizable,
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
            .imageCount = settingsConfig.imageCount,
            .synchronise = settingsConfig.vsync,
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

        renderer::RenderPassCreateInfo renderPassCreateInfo = gameInstance_->makeRequiredRenderPass();

        renderPass_ = data::makeUnique<renderer::RenderPass>(renderPassCreateInfo);

        imageCount_ = swapchain_->imageCount();
        imageIndex_ = 0;

        frameCount_ = std::min(imageCount_, settingsConfig.renderAheadLimit);
        frameIndex_ = 0;

        renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
            .device = device_.ref(),
        };

        renderer::FenceCreateInfo fenceCreateInfo = {
            .device = device_.ref(),
            .createFlags = renderer::FenceCreateFlags::START_SIGNALED,
        };

        std::span<renderer::ImageView> swapchainImages = swapchain_->images();

        presentSemaphores_.reserve(imageCount_);
        framebuffers_.reserve(imageCount_);

        for (std::size_t i = 0; i < imageCount_; i++) {
            renderer::FramebufferCreateInfo framebufferCreateInfo = {
                .device = device_.ref(),
                .renderPass = renderPass_.ref(),
                .imageViews = {swapchainImages[i]},
            };

            framebuffers_.emplace_back(framebufferCreateInfo);
            presentSemaphores_.emplace_back(semaphoreCreateInfo);
        }

        acquireSemaphores_.reserve(frameCount_);
        inFlightFences_.reserve(frameCount_);

        for (std::size_t i = 0; i < frameCount_; i++) {
            acquireSemaphores_.emplace_back(semaphoreCreateInfo);
            inFlightFences_.emplace_back(fenceCreateInfo);
        }

        renderer::CommandBufferCreateInfo commandBufferCreateInfo = {
            .count = frameCount_,
        };

        commandBuffers_ = graphicsCommandPool_->allocateCommandBuffers(commandBufferCreateInfo);
    }

    Program::~Program() {
        if (device_) {
            device_->waitIdle();
        }
    }

    void Program::run() {
        gameInstance_->start();

        bool running = true;
        bool resized = false;

        while (running) {
            manageEvents(running);
            acquireImage(resized);

            gameInstance_->update();
            gameInstance_->render();

            presentImage();
        }

        gameInstance_->close();
    }

    SettingsConfig Program::loadSettings() {
        SettingsConfig settingsConfig;

        std::ifstream settingsFile("config/settings.json");
        std::string settingsSource(std::istreambuf_iterator<char>(settingsFile), {});

        nlohmann::json settings = nlohmann::json::parse(settingsSource);

        settingsConfig.displaySize.width = settings["display"]["width"].get<std::uint32_t>();
        settingsConfig.displaySize.height = settings["display"]["height"].get<std::uint32_t>();
        settingsConfig.imageCount = settings["graphics"]["imageCount"].get<std::uint32_t>();
        settingsConfig.renderAheadLimit = settings["graphics"]["renderAheadLimit"].get<std::uint32_t>();
        settingsConfig.resizable = settings["display"]["resizable"].get<bool>();
        settingsConfig.vsync = settings["graphics"]["vsync"].get<bool>();

        std::string displayMode = settings["display"]["mode"].get<std::string>();

        if (displayMode == "windowed") {
            settingsConfig.displayMode = WindowVisibility::WINDOWED;
        }
        else if (displayMode == "fullscreen") {
            settingsConfig.displayMode = WindowVisibility::FULLSCREEN;
        }
        else {
            throw std::runtime_error("Bad value for \"display.mode\" in file \"config/settings.json\"");
        }

        return settingsConfig;
    }

    void Program::applySettings(const SettingsConfig& config) {
        device_->waitIdle();

        window_->setExtent(config.displaySize);
        window_->setVisibility(config.displayMode);

        swapchainRecreateImageCount_ = config.imageCount;
        swapchainRecreateFrameCount_ = config.renderAheadLimit;
        swapchainRecreateSynchronise_ = config.vsync;
        explicitSwapchainRecreate_ = true;
    }

    renderer::Device& Program::device() {
        return device_.ref();
    }

    renderer::Swapchain& Program::swapchain() {
        return swapchain_.ref();
    }

    renderer::RenderPass& Program::renderPass() {
        return renderPass_.ref();
    }

    renderer::CommandPool& Program::transferCommandPool() {
        return transferCommandPool_.ref();
    }

    renderer::Queue& Program::transferQueue() {
        return transferQueue_.get();
    }

    renderer::Queue& Program::graphicsQueue() {
        return graphicsQueue_.get();
    }

    renderer::CommandBuffer& Program::currentCommandBuffer() {
        return commandBuffers_[frameIndex_];
    }

    renderer::Framebuffer& Program::currentFramebuffer() {
        return framebuffers_[imageIndex_];
    }

    renderer::Semaphore& Program::currentAcquireSemaphore() {
        return acquireSemaphores_[frameIndex_];
    }

    renderer::Semaphore& Program::currentPresentSemaphore() {
        return presentSemaphores_[imageIndex_];
    }

    renderer::Fence& Program::currentFence() {
        return inFlightFences_[frameIndex_];
    }

    void Program::manageBindings(const WindowKeyPressedEventInfo& pressEvent) {
        switch (pressEvent.key) {
            case Key::GRAVE_ACCENT:
                applySettings(loadSettings());
                break;

            default:
                break;
        }
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
                    manageBindings(event.info.keyPress);
                    break;

                default:
                    break;
            }
        }
    }

    void Program::acquireImage(bool& resized) {
        resized = false;

        if (explicitSwapchainRecreate_) {
            explicitSwapchainRecreate_ = false;

            renderer::SwapchainRecreateInfo swapchainRecreateInfo = {
                .imageCount = swapchainRecreateImageCount_,
                .synchronise = swapchainRecreateSynchronise_,
            };

            device_->waitIdle();
            swapchain_->recreate(swapchainRecreateInfo);

            resized = true;

            frameCount_ = std::min(imageCount_, swapchainRecreateFrameCount_);
            frameIndex_ = std::min(frameIndex_, frameCount_ - 1);

            acquireSemaphores_.clear();
            inFlightFences_.clear();

            renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
                .device = device_.ref(),
            };

            renderer::FenceCreateInfo fenceCreateInfo = {
                .device = device_.ref(),
                .createFlags = renderer::FenceCreateFlags::START_SIGNALED,
            };

            acquireSemaphores_.reserve(frameCount_);
            inFlightFences_.reserve(frameCount_);

            for (std::size_t i = 0; i < frameCount_; i++) {
                acquireSemaphores_.emplace_back(semaphoreCreateInfo);
                inFlightFences_.emplace_back(fenceCreateInfo);
            }

            graphicsCommandPool_->destroyCommandBuffers(commandBuffers_);

            renderer::CommandBufferCreateInfo commandBufferCreateInfo = {
                .count = frameCount_,
            };

            commandBuffers_ = graphicsCommandPool_->allocateCommandBuffers(commandBufferCreateInfo);
        }

        renderer::Semaphore& acquireSemaphore = acquireSemaphores_[frameIndex_];
        renderer::Fence& inFlightFence = inFlightFences_[frameIndex_];

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

            imageIndex_ = swapchain_->acquireNextImage(acquireSemaphore);

            if (!swapchain_->shouldRecreate()) {
                break;
            }
        }

        if (resized) {
            framebuffers_.clear();
            presentSemaphores_.clear();

            imageCount_ = swapchain_->imageCount();

            renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
                .device = device_.ref(),
            };

            std::span<renderer::ImageView> swapchainImages = swapchain_->images();

            for (std::size_t i = 0; i < imageCount_; i++) {
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
        renderer::Semaphore& presentSemaphore = presentSemaphores_[imageIndex_];

        swapchain_->presentNextImage(presentSemaphore);

        frameIndex_ = (frameIndex_ + 1) % frameCount_;
    }
}