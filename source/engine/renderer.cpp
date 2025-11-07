#include <engine/renderer.hpp>

void engine::Renderer::create(vulkanite::window::Window& window) {
    vulkanite::renderer::InstanceCreateInfo instanceCreateInfo = {
        .applicationName = window.getTitle(),
        .applicationVersionMajor = 0,
        .applicationVersionMinor = 0,
        .applicationVersionPatch = 1,
        .requestDebug = true,
    };

    vulkanite::renderer::SurfaceCreateInfo surfaceCreateInfo = {
        .instance = instance_,
        .window = window,
    };

    vulkanite::renderer::QueueCreateInfo graphicsQueueInfo = {
        .flags = vulkanite::renderer::QueueFlags::GRAPHICS,
        .surface = nullptr,
    };

    vulkanite::renderer::QueueCreateInfo transferQueueInfo = {
        .flags = vulkanite::renderer::QueueFlags::TRANSFER,
        .surface = nullptr,
    };

    vulkanite::renderer::QueueCreateInfo presentQueueInfo = {
        .flags = vulkanite::renderer::QueueFlags::PRESENT,
        .surface = &surface_,
    };

    vulkanite::renderer::DeviceCreateInfo deviceCreateInfo = {
        .instance = instance_,
        .queues = {
            graphicsQueueInfo,
            transferQueueInfo,
            presentQueueInfo,
        },
    };

    instance_.create(instanceCreateInfo);
    surface_.create(surfaceCreateInfo);
    device_.create(deviceCreateInfo);

    std::span<vulkanite::renderer::Queue> queues = device_.getQueues();

    graphicsQueue_ = queues[0];
    transferQueue_ = queues[1];
    presentQueue_ = queues[2];

    // TODO: make configurable again
    vulkanite::renderer::SwapchainCreateInfo swapchainCreateInfo = {
        .surface = surface_,
        .device = device_,
        .presentQueue = presentQueue_,
        .requestedImageCount = 3,
        .shouldSynchronise = true,
    };

    auto result = swapchain_.create(swapchainCreateInfo);

    if (result == vulkanite::renderer::SwapchainResult::FAILED) {
        throw std::runtime_error("Call failed: vulkanite::renderer::Renderer::Renderer(): Failed to create swapchain");
    }

    vulkanite::renderer::CommandPoolCreateInfo commandPoolCreateInfo = {
        .device = device_,
        .queue = graphicsQueue_,
    };

    vulkanite::renderer::RenderPassCreateInfo renderPassCreateInfo = {
        .device = device_,
        .depthStencilAttachments = {},
        .colourAttachments = {
            vulkanite::renderer::ColourAttachmentInfo{
                .format = swapchain_.getFormat(),
                .initialLayout = vulkanite::renderer::ImageLayout::UNDEFINED,
                .finalLayout = vulkanite::renderer::ImageLayout::PRESENT_SOURCE,
                .operations = {
                    .load = vulkanite::renderer::LoadOperation::CLEAR,
                    .store = vulkanite::renderer::StoreOperation::STORE,
                },
            },
        },
        .subpasses = {
            vulkanite::renderer::SubpassInfo{
                .colourAttachmentInputIndices = {},
                .colourAttachmentOutputIndices = {
                    0,
                },
                .depthStencilIndex = {},
            },
        },
        .subpassDependencies = {},
        .sampleCount = 1,
    };

    commandPool_.create(commandPoolCreateInfo);
    renderPass_.create(renderPassCreateInfo);

    imageCounter_.count = swapchain_.getImageCount();
    imageCounter_.index = 0;

    // TODO: make configurable again
    frameCounter_.count = std::min(imageCounter_.count, 3u);
    frameCounter_.index = 0;

    vulkanite::renderer::FenceCreateInfo fenceCreateInfo = {
        .device = device_,
        .createFlags = vulkanite::renderer::FenceCreateFlags::START_SIGNALLED,
    };

    auto swapchainImageViews = swapchain_.getImageViews();

    presentSemaphores_.reserve(imageCounter_.count);
    framebuffers_.reserve(imageCounter_.count);

    for (std::uint64_t i = 0; i < imageCounter_.count; i++) {
        vulkanite::renderer::FramebufferCreateInfo framebufferCreateInfo = {
            .device = device_,
            .renderPass = renderPass_,
            .imageViews = {swapchainImageViews[i]},
        };

        auto& framebuffer = framebuffers_.emplace_back();
        auto& semaphore = presentSemaphores_.emplace_back();

        framebuffer.create(framebufferCreateInfo);
        semaphore.create(device_);
    }

    acquireSemaphores_.reserve(frameCounter_.count);
    inFlightFences_.reserve(frameCounter_.count);

    for (std::uint64_t i = 0; i < frameCounter_.count; i++) {
        auto& semaphore = acquireSemaphores_.emplace_back();
        auto& fence = inFlightFences_.emplace_back();

        semaphore.create(device_);
        fence.create(fenceCreateInfo);
    }

    commandBuffers_ = commandPool_.allocateCommandBuffers(frameCounter_.count);
}

engine::Renderer::~Renderer() {
    for (std::uint64_t i = 0; i < frameCounter_.count; i++) {
        acquireSemaphores_[i].destroy();
        inFlightFences_[i].destroy();
    }

    for (std::uint64_t i = 0; i < imageCounter_.count; i++) {
        framebuffers_[i].destroy();
        presentSemaphores_[i].destroy();
    }

    commandPool_.destroy();
    renderPass_.destroy();
    swapchain_.destroy();
    device_.destroy();
    surface_.destroy();
    instance_.destroy();
}

void engine::Renderer::acquireImage(const std::vector<vulkanite::renderer::Fence>& fences) {
    vulkanite::renderer::Fence& inFlightFence = inFlightFences_[frameCounter_.index];
    vulkanite::renderer::Semaphore& acquireSemaphore = acquireSemaphores_[frameCounter_.index];

    if (!awaitRestore_) {
        std::vector<vulkanite::renderer::Fence> fencesToUse;

        fencesToUse.reserve(fences.size() + 1);
        fencesToUse.insert(fencesToUse.end(), fences.begin(), fences.end());
        fencesToUse.push_back(inFlightFence);

        device_.waitForFences(fencesToUse);
        device_.resetFences(fencesToUse);
    }

    resized_ = false;

    while (true) {
        if (swapchain_.shouldRecreate()) {
            // TODO: make configurable again
            vulkanite::renderer::SwapchainCreateInfo swapchainCreateInfo = {
                .surface = surface_,
                .device = device_,
                .presentQueue = presentQueue_,
                .oldSwapchain = &swapchain_,
                .requestedImageCount = 3,
                .shouldSynchronise = true,
            };

            device_.waitIdle();

            vulkanite::renderer::Swapchain newSwapchain;
            vulkanite::renderer::SwapchainResult result = newSwapchain.create(swapchainCreateInfo);

            if (result == vulkanite::renderer::SwapchainResult::BUSY) {
                awaitRestore_ = true;
                resized_ = false;

                return;
            }
            else if (result == vulkanite::renderer::SwapchainResult::FAILED) {
                throw std::runtime_error("Call failed: vulkanite::renderer::Renderer::acquireImage(): Failed to recreate swapchain");
            }

            swapchain_ = newSwapchain;

            resized_ = true;
        }

        swapchain_.acquireNextImage(acquireSemaphore);
        imageCounter_.index = swapchain_.getImageIndex();

        if (!swapchain_.shouldRecreate()) {
            break;
        }
    }

    if (resized_) {
        imageCounter_.count = swapchain_.getImageCount();

        framebuffers_.clear();
        presentSemaphores_.clear();

        auto swapchainImageViews = swapchain_.getImageViews();

        presentSemaphores_.reserve(imageCounter_.count);
        framebuffers_.reserve(imageCounter_.count);

        for (std::uint64_t i = 0; i < imageCounter_.count; i++) {
            vulkanite::renderer::FramebufferCreateInfo framebufferCreateInfo = {
                .device = device_,
                .renderPass = renderPass_,
                .imageViews = {swapchainImageViews[i]},
            };

            auto& framebuffer = framebuffers_.emplace_back();
            auto& semaphore = presentSemaphores_.emplace_back();

            framebuffer.create(framebufferCreateInfo);
            semaphore.create(device_);
        }
    }
}

void engine::Renderer::presentImage() {
    vulkanite::renderer::Semaphore& presentSemaphore = presentSemaphores_[imageCounter_.index];

    swapchain_.presentNextImage(presentSemaphore);

    frameCounter_.index = (frameCounter_.index + 1) % frameCounter_.count;
}
