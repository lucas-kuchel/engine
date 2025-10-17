#include "app/configuration.hpp"
#include "renderer/semaphore.hpp"
#include "renderer/surface.hpp"
#include "renderer/swapchain.hpp"
#include <app/program.hpp>

namespace app {
    Program::Program() {
        game::loadSettings(settings_);

        context_ = std::make_unique<Context>();

        WindowCreateInfo windowCreateInfo = {
            .context = *context_.get(),
            .extent = {settings_.display.size.x, settings_.display.size.y},
            .title = "Game",
            .visibility = settings_.display.mode,
            .resizable = settings_.display.resizable,
        };

        window_ = std::make_unique<Window>(windowCreateInfo);

        renderer::InstanceCreateInfo instanceCreateInfo = {
            .applicationName = window_->getTitle(),
            .applicationVersionMajor = 0,
            .applicationVersionMinor = 0,
            .applicationVersionPatch = 1,
            .engineName = "engine",
            .engineVersionMajor = 0,
            .engineVersionMinor = 0,
            .engineVersionPatch = 1,
            .requestDebug = true,
        };

        instance_ = renderer::Instance::create(instanceCreateInfo);

        renderer::SurfaceCreateInfo surfaceCreateInfo = {
            .instance = instance_,
            .window = *window_.get(),
        };

        surface_ = renderer::Surface::create(surfaceCreateInfo);

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
            .surface = &surface_,
        };

        renderer::DeviceCreateInfo deviceCreateInfo = {
            .instance = instance_,
            .queues = {
                graphicsQueueInfo,
                transferQueueInfo,
                presentQueueInfo,
            },
        };

        device_ = renderer::Device::create(deviceCreateInfo);

        std::span<renderer::Queue> queues = renderer::Device::getQueues(device_);

        graphicsQueue_ = queues[0];
        transferQueue_ = queues[1];
        presentQueue_ = queues[2];

        renderer::SwapchainCreateInfo swapchainCreateInfo = {
            .surface = surface_,
            .device = device_,
            .presentQueue = presentQueue_,
            .imageCount = settings_.graphics.imageCount,
            .synchronise = settings_.graphics.vsync,
        };

        auto [newSwapchain, swapchainResult] = renderer::Swapchain::create(swapchainCreateInfo);

        if (swapchainResult == renderer::SwapchainCreateResult::FAILED) {
            throw std::runtime_error("Call failed: app::Program::Program(): Failed to create swapchain");
        }

        swapchain_ = newSwapchain;

        renderer::CommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
            .device = device_,
            .queue = graphicsQueue_,
        };

        renderer::CommandPoolCreateInfo transferCommandPoolCreateInfo = {
            .device = device_,
            .queue = transferQueue_,
        };

        graphicsCommandPool_ = renderer::CommandPool::create(graphicsCommandPoolCreateInfo);
        transferCommandPool_ = renderer::CommandPool::create(transferCommandPoolCreateInfo);

        renderer::RenderPassCreateInfo renderPassCreateInfo = {
            .device = device_,
            .depthStencilAttachments = {
                renderer::DepthStencilInfo{
                    .format = renderer::ImageFormat::DEPTH_ONLY,
                    .initialLayout = renderer::ImageLayout::UNDEFINED,
                    .finalLayout = renderer::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .depthOperations = {
                        .load = renderer::LoadOperation::CLEAR,
                        .store = renderer::StoreOperation::STORE,
                    },
                    .stencilOperations = {
                        .load = renderer::LoadOperation::DONT_CARE,
                        .store = renderer::StoreOperation::DONT_CARE,
                    },
                },
            },
            .colourAttachments = {
                renderer::ColourAttachmentInfo{
                    .format = renderer::Swapchain::getFormat(swapchain_),
                    .initialLayout = renderer::ImageLayout::UNDEFINED,
                    .finalLayout = renderer::ImageLayout::PRESENT_SOURCE,
                    .operations = {
                        .load = renderer::LoadOperation::CLEAR,
                        .store = renderer::StoreOperation::STORE,
                    },
                },
            },
            .subpasses = {
                renderer::SubpassInfo{
                    .colourAttachmentInputIndices = {},
                    .colourAttachmentOutputIndices = {
                        0,
                    },
                    .depthStencilIndex = 0,
                },
            },
            .subpassDependencies = {},
            .sampleCount = 1,
        };

        renderPass_ = renderer::RenderPass::create(renderPassCreateInfo);

        imageCounter_.count = renderer::Swapchain::getImageCount(swapchain_);
        imageCounter_.index = 0;

        frameCounter_.count = std::min(imageCounter_.count, settings_.graphics.renderAheadLimit);
        frameCounter_.index = 0;

        renderer::FenceCreateInfo fenceCreateInfo = {
            .device = device_,
            .createFlags = renderer::FenceCreateFlags::START_SIGNALED,
        };

        auto swapchainImageViews = renderer::Swapchain::getImageViews(swapchain_);
        auto swapchainExtent = renderer::Swapchain::getExtent(swapchain_);

        presentSemaphores_.reserve(imageCounter_.count);
        depthImages_.reserve(imageCounter_.count);
        depthImageViews_.reserve(imageCounter_.count);
        framebuffers_.reserve(imageCounter_.count);

        renderer::ImageCreateInfo depthBufferImageCreateInfo = {
            .device = device_,
            .type = renderer::ImageType::IMAGE_2D,
            .format = renderer::ImageFormat::DEPTH_ONLY,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT,
            .extent = {swapchainExtent, 1},
            .sampleCount = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
        };

        for (std::size_t i = 0; i < imageCounter_.count; i++) {
            depthImages_.push_back(renderer::Image());

            auto& image = depthImages_.back();

            image = renderer::Image::create(depthBufferImageCreateInfo);

            renderer::ImageViewCreateInfo depthBufferImageViewCreateInfo = {
                .image = depthImages_[i],
                .type = renderer::ImageViewType::IMAGE_2D,
                .aspectFlags = renderer::ImageAspectFlags::DEPTH,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            depthImageViews_.push_back(renderer::ImageView());

            auto& imageView = depthImageViews_.back();

            imageView = renderer::ImageView::create(depthBufferImageViewCreateInfo);

            renderer::FramebufferCreateInfo framebufferCreateInfo = {
                .device = device_,
                .renderPass = renderPass_,
                .imageViews = {swapchainImageViews[i], depthImageViews_[i]},
            };

            framebuffers_.push_back(renderer::Framebuffer());
            presentSemaphores_.push_back(renderer::Semaphore());

            auto& framebuffer = framebuffers_.back();
            auto& semaphore = presentSemaphores_.back();

            framebuffer = renderer::Framebuffer::create(framebufferCreateInfo);
            semaphore = renderer::Semaphore::create(device_);
        }

        acquireSemaphores_.reserve(frameCounter_.count);
        inFlightFences_.reserve(frameCounter_.count);

        for (std::size_t i = 0; i < frameCounter_.count; i++) {
            acquireSemaphores_.push_back(renderer::Semaphore());
            inFlightFences_.push_back(renderer::Fence());

            auto& semaphore = acquireSemaphores_.back();
            auto& fence = inFlightFences_.back();

            semaphore = renderer::Semaphore::create(device_);
            fence = renderer::Fence::create(fenceCreateInfo);
        }

        commandBuffers_ = renderer::CommandPool::allocateCommandBuffers(graphicsCommandPool_, frameCounter_.count);

        run();
    }

    Program::~Program() {
        renderer::Device::waitIdle(device_);

        for (std::size_t i = 0; i < imageCounter_.count; i++) {
            renderer::Image::destroy(depthImages_[i]);
            renderer::ImageView::destroy(depthImageViews_[i]);
            renderer::Framebuffer::destroy(framebuffers_[i]);
            renderer::Semaphore::destroy(presentSemaphores_[i]);
        }

        for (std::size_t i = 0; i < frameCounter_.count; i++) {
            renderer::Semaphore::destroy(acquireSemaphores_[i]);
            renderer::Fence::destroy(inFlightFences_[i]);
        }

        acquireSemaphores_.clear();
        inFlightFences_.clear();
        framebuffers_.clear();
        presentSemaphores_.clear();
        depthImages_.clear();
        depthImageViews_.clear();

        renderer::CommandPool::destroy(transferCommandPool_);
        renderer::CommandPool::destroy(graphicsCommandPool_);

        renderer::RenderPass::destroy(renderPass_);

        renderer::Swapchain::destroy(swapchain_);
        renderer::Device::destroy(device_);
        renderer::Surface::destroy(surface_);
        renderer::Instance::destroy(instance_);
    }

    void Program::acquireImage() {
        renderer::Fence& inFlightFence = inFlightFences_[frameCounter_.index];
        renderer::Semaphore& acquireSemaphore = acquireSemaphores_[frameCounter_.index];

        if (!awaitRestore_) {
            renderer::Device::waitForFences(device_, {inFlightFence, stagingBufferFence_});
            renderer::Device::resetFences(device_, {inFlightFence, stagingBufferFence_});
        }

        resized_ = false;

        while (true) {
            if (renderer::Swapchain::shouldRecreate(swapchain_)) {
                renderer::SwapchainCreateInfo swapchainCreateInfo = {
                    .surface = surface_,
                    .device = device_,
                    .presentQueue = presentQueue_,
                    .imageCount = settings_.graphics.imageCount,
                    .synchronise = settings_.graphics.vsync,
                    .oldSwapchain = &swapchain_,
                };

                renderer::Device::waitIdle(device_);

                auto [newSwapchain, swapchainResult] = renderer::Swapchain::create(swapchainCreateInfo);

                if (swapchainResult == renderer::SwapchainCreateResult::PENDING) {
                    awaitRestore_ = true;
                    resized_ = false;

                    return;
                }
                else if (swapchainResult == renderer::SwapchainCreateResult::FAILED) {
                    throw std::runtime_error("Call failed: app::Program::acquireImage(): Failed to recreate swapchain");
                }

                swapchain_ = newSwapchain;

                resized_ = true;
            }

            renderer::Swapchain::acquireNextImage(swapchain_, acquireSemaphore);
            imageCounter_.index = renderer::Swapchain::getImageIndex(swapchain_);

            if (!renderer::Swapchain::shouldRecreate(swapchain_)) {
                break;
            }
        }

        if (resized_) {
            imageCounter_.count = renderer::Swapchain::getImageCount(swapchain_);
            frameCounter_.count = std::min(imageCounter_.count, settings_.graphics.renderAheadLimit);
            frameCounter_.index = std::min(frameCounter_.index, frameCounter_.count - 1);

            for (auto& semaphore : presentSemaphores_) {
                renderer::Semaphore::destroy(semaphore);
            }

            for (auto& image : depthImages_) {
                renderer::Image::destroy(image);
            }

            for (auto& imageView : depthImageViews_) {
                renderer::ImageView::destroy(imageView);
            }

            for (auto& framebuffer : framebuffers_) {
                renderer::Framebuffer::destroy(framebuffer);
            }

            framebuffers_.clear();
            presentSemaphores_.clear();
            depthImages_.clear();
            depthImageViews_.clear();

            renderer::CommandPool::destroyCommandBuffers(graphicsCommandPool_, commandBuffers_);
            commandBuffers_ = renderer::CommandPool::allocateCommandBuffers(graphicsCommandPool_, frameCounter_.count);

            auto swapchainImageViews = renderer::Swapchain::getImageViews(swapchain_);
            auto swapchainExtent = renderer::Swapchain::getExtent(swapchain_);

            presentSemaphores_.reserve(imageCounter_.count);
            depthImages_.reserve(imageCounter_.count);
            depthImageViews_.reserve(imageCounter_.count);
            framebuffers_.reserve(imageCounter_.count);

            renderer::ImageCreateInfo depthBufferImageCreateInfo = {
                .device = device_,
                .type = renderer::ImageType::IMAGE_2D,
                .format = renderer::ImageFormat::DEPTH_ONLY,
                .memoryType = renderer::MemoryType::DEVICE_LOCAL,
                .usageFlags = renderer::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT,
                .extent = {swapchainExtent, 1},
                .sampleCount = 1,
                .mipLevels = 1,
                .arrayLayers = 1,
            };

            for (std::size_t i = 0; i < imageCounter_.count; i++) {
                depthImages_.push_back(renderer::Image());

                auto& image = depthImages_.back();

                image = renderer::Image::create(depthBufferImageCreateInfo);

                renderer::ImageViewCreateInfo depthBufferImageViewCreateInfo = {
                    .image = depthImages_[i],
                    .type = renderer::ImageViewType::IMAGE_2D,
                    .aspectFlags = renderer::ImageAspectFlags::DEPTH,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                };

                depthImageViews_.push_back(renderer::ImageView());

                auto& imageView = depthImageViews_.back();

                imageView = renderer::ImageView::create(depthBufferImageViewCreateInfo);

                renderer::FramebufferCreateInfo framebufferCreateInfo = {
                    .device = device_,
                    .renderPass = renderPass_,
                    .imageViews = {swapchainImageViews[i], depthImageViews_[i]},
                };

                framebuffers_.push_back(renderer::Framebuffer());
                presentSemaphores_.push_back(renderer::Semaphore());

                auto& framebuffer = framebuffers_.back();
                auto& semaphore = presentSemaphores_.back();

                framebuffer = renderer::Framebuffer::create(framebufferCreateInfo);
                semaphore = renderer::Semaphore::create(device_);
            }
        }
    }

    void Program::presentImage() {
        renderer::Semaphore& presentSemaphore = presentSemaphores_[imageCounter_.index];

        renderer::Swapchain::presentNextImage(swapchain_, presentSemaphore);

        frameCounter_.index = (frameCounter_.index + 1) % frameCounter_.count;
    }

    void Program::manageEvents() {
        context_->pollEvents();

        while (window_->hasEvents()) {
            WindowEvent event = window_->getNextEvent();

            switch (event.type) {
                case WindowEventType::CLOSED:
                    running_ = false;
                    break;

                case WindowEventType::KEY_PRESSED:
                    keysPressed_[keyIndex(event.info.keyPress.key)] = true;
                    keysHeld_[keyIndex(event.info.keyPress.key)] = true;
                    keysReleased_[keyIndex(event.info.keyPress.key)] = false;

                    switch (event.info.keyPress.key) {
                        case Key::F11: {
#if !defined(PLATFORM_APPLE)
                            if (window_->getVisibility() != WindowVisibility::FULLSCREEN) {
                                window_->setVisibility(WindowVisibility::FULLSCREEN);
                            }
                            else {
                                window_->setVisibility(WindowVisibility::WINDOWED);
                            }
#endif

                            break;
                        }
                        default:
                            break;
                    }
                    break;

                case WindowEventType::KEY_RELEASED:
                    keysPressed_[keyIndex(event.info.keyRelease.key)] = false;
                    keysHeld_[keyIndex(event.info.keyRelease.key)] = false;
                    keysReleased_[keyIndex(event.info.keyRelease.key)] = true;
                    break;

                default:
                    break;
            }
        }
    }

    void Program::run() {
        start();

        while (running_) {
            for (std::size_t i = 0; i < keysPressed_.size(); i++) {
                keysPressed_[i] = keysPressed_[i] && !keysHeld_[i];
                keysReleased_[i] = keysReleased_[i] && keysHeld_[i];
            }

            manageEvents();
            acquireImage();

            if (awaitRestore_) {
                if (window_->getVisibility() == WindowVisibility::MINIMISED) {
                    continue;
                }

                awaitRestore_ = false;
            }

            update();
            render();

            presentImage();
        }

        close();
    }

    std::size_t Program::keyIndex(Key key) {
        return static_cast<std::size_t>(key);
    }
}