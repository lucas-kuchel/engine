#include <renderer/renderer.hpp>

renderer::Renderer::Renderer(app::Window& window) {
    renderer::InstanceCreateInfo instanceCreateInfo = {
        .applicationName = window.getTitle(),
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
        .window = window,
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

    // TODO: make configurable again
    renderer::SwapchainCreateInfo swapchainCreateInfo = {
        .surface = surface_,
        .device = device_,
        .presentQueue = presentQueue_,
        .imageCount = 3,
        .synchronise = false,
    };

    auto [newSwapchain, swapchainResult] = renderer::Swapchain::create(swapchainCreateInfo);

    if (swapchainResult == renderer::SwapchainCreateResult::FAILED) {
        throw std::runtime_error("Call failed: renderer::Renderer::Renderer(): Failed to create swapchain");
    }

    swapchain_ = newSwapchain;

    renderer::CommandPoolCreateInfo commandPoolCreateInfo = {
        .device = device_,
        .queue = graphicsQueue_,
    };

    commandPool_ = renderer::CommandPool::create(commandPoolCreateInfo);

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

    // TODO: make configurable again
    frameCounter_.count = std::min(imageCounter_.count, 3u);
    frameCounter_.index = 0;

    renderer::FenceCreateInfo fenceCreateInfo = {
        .device = device_,
        .createFlags = renderer::FenceCreateFlags::START_SIGNALLED,
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

    for (std::uint64_t i = 0; i < imageCounter_.count; i++) {
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

    for (std::uint64_t i = 0; i < frameCounter_.count; i++) {
        acquireSemaphores_.push_back(renderer::Semaphore());
        inFlightFences_.push_back(renderer::Fence());

        auto& semaphore = acquireSemaphores_.back();
        auto& fence = inFlightFences_.back();

        semaphore = renderer::Semaphore::create(device_);
        fence = renderer::Fence::create(fenceCreateInfo);
    }

    commandBuffers_ = renderer::CommandPool::allocateCommandBuffers(commandPool_, frameCounter_.count);
}

renderer::Renderer::~Renderer() {
    for (std::uint64_t i = 0; i < imageCounter_.count; i++) {
        renderer::Image::destroy(depthImages_[i]);
        renderer::ImageView::destroy(depthImageViews_[i]);
        renderer::Framebuffer::destroy(framebuffers_[i]);
        renderer::Semaphore::destroy(presentSemaphores_[i]);
    }

    for (std::uint64_t i = 0; i < frameCounter_.count; i++) {
        renderer::Semaphore::destroy(acquireSemaphores_[i]);
        renderer::Fence::destroy(inFlightFences_[i]);
    }

    acquireSemaphores_.clear();
    inFlightFences_.clear();
    framebuffers_.clear();
    presentSemaphores_.clear();
    depthImages_.clear();
    depthImageViews_.clear();

    renderer::CommandPool::destroy(commandPool_);
    renderer::RenderPass::destroy(renderPass_);
    renderer::Swapchain::destroy(swapchain_);
    renderer::Device::destroy(device_);
    renderer::Surface::destroy(surface_);
    renderer::Instance::destroy(instance_);
}

void renderer::Renderer::acquireImage(const std::vector<Fence>& fences) {
    renderer::Fence& inFlightFence = inFlightFences_[frameCounter_.index];
    renderer::Semaphore& acquireSemaphore = acquireSemaphores_[frameCounter_.index];

    if (!awaitRestore_) {
        std::vector<Fence> fencesToUse;

        fencesToUse.reserve(fences.size() + 1);
        fencesToUse.insert(fencesToUse.end(), fences.begin(), fences.end());
        fencesToUse.push_back(inFlightFence);

        renderer::Device::waitForFences(device_, fencesToUse);
        renderer::Device::resetFences(device_, fencesToUse);
    }

    resized_ = false;

    while (true) {
        if (renderer::Swapchain::shouldRecreate(swapchain_)) {

            // TODO: make configurable again
            renderer::SwapchainCreateInfo swapchainCreateInfo = {
                .surface = surface_,
                .device = device_,
                .presentQueue = presentQueue_,
                .imageCount = 3,
                .synchronise = false,
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
                throw std::runtime_error("Call failed: renderer::Renderer::acquireImage(): Failed to recreate swapchain");
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

        // TODO: make configurable again
        frameCounter_.count = std::min(imageCounter_.count, 3u);
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

        renderer::CommandPool::destroyCommandBuffers(commandPool_, commandBuffers_);
        commandBuffers_ = renderer::CommandPool::allocateCommandBuffers(commandPool_, frameCounter_.count);

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

        for (std::uint64_t i = 0; i < imageCounter_.count; i++) {
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

void renderer::Renderer::presentImage() {
    renderer::Semaphore& presentSemaphore = presentSemaphores_[imageCounter_.index];

    renderer::Swapchain::presentNextImage(swapchain_, presentSemaphore);

    frameCounter_.index = (frameCounter_.index + 1) % frameCounter_.count;
}