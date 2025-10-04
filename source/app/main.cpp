#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/fence.hpp>
#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/pipeline.hpp>
#include <renderer/resources/semaphore.hpp>
#include <renderer/resources/shader.hpp>

#include <filesystem/file.hpp>

#include <exception>
#include <print>

void run() {
    std::uint32_t imageCount = 3;
    std::uint32_t frameCount = 3;
    std::uint32_t frameIndex = 0;

    bool synchronise = false;

    app::Context context;

    app::WindowCreateInfo windowCreateInfo = {
        .context = context,
        .extent = {1280, 720},
        .title = "Vulkan Test - Engine",
        .visibility = app::WindowVisibility::WINDOWED,
        .resizable = true,
    };

    app::Window window(windowCreateInfo);

    renderer::InstanceCreateInfo instanceCreateInfo = {
        .applicationName = window.getTitle(),
        .applicationVersion = data::Version(0, 0, 1),
        .engineName = "Engine",
        .engineVersion = data::Version(0, 0, 1),
        .requestDebug = true,
    };

    renderer::Instance instance(instanceCreateInfo);

    renderer::SurfaceCreateInfo surfaceCreateInfo = {
        .instance = instance,
        .window = window,
    };

    renderer::Surface surface(surfaceCreateInfo);

    renderer::DeviceCreateInfo deviceCreateInfo = {
        .instance = instance,
        .queues = {
            renderer::QueueCreateInfo{
                .flags = renderer::QueueFlags::GRAPHICS | renderer::QueueFlags::TRANSFER,
                .surface = nullptr,
            },
            renderer::QueueCreateInfo{
                .flags = renderer::QueueFlags::PRESENT,
                .surface = surface,
            },
        },
    };

    renderer::Device device(deviceCreateInfo);

    std::span<renderer::Queue> queues = device.getQueues();

    renderer::Queue& renderQueue = queues[0];
    renderer::Queue& presentQueue = queues[1];

    renderer::SwapchainCreateInfo swapchainCreateInfo = {
        .surface = surface,
        .device = device,
        .presentQueue = presentQueue,
        .imageCount = imageCount,
        .synchronise = synchronise,
    };

    renderer::Swapchain swapchain(swapchainCreateInfo);

    imageCount = swapchain.getImageCount();
    frameCount = std::min(imageCount, frameCount);

    renderer::CommandPoolCreateInfo commandPoolCreateInfo = {
        .device = device,
        .queue = renderQueue,
    };

    renderer::CommandPool commandPool(commandPoolCreateInfo);

    renderer::CommandBufferCreateInfo commandBufferCreateInfo = {
        .count = frameCount,
    };

    std::vector<renderer::CommandBuffer> commandBuffers = commandPool.allocateCommandBuffers(commandBufferCreateInfo);

    renderer::RenderPassCreateInfo renderPassCreateInfo = {
        .device = device,
        .depthStencilAttachments = {},
        .colourAttachments = {
            renderer::ColourAttachmentInfo{
                .format = swapchain.getImageFormat(),
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
                .depthStencilIndex = {},
            },
        },
        .subpassDependencies = {},
        .sampleCount = 1,
    };

    renderer::RenderPass renderPass(renderPassCreateInfo);

    std::vector<renderer::Framebuffer> framebuffers;
    std::vector<renderer::Semaphore> acquireSemaphores;
    std::vector<renderer::Semaphore> submitSemaphores;
    std::vector<renderer::Fence> fences;

    data::NullableReferenceList<renderer::Fence> perImageFences;

    acquireSemaphores.reserve(frameCount);
    fences.reserve(frameCount);
    submitSemaphores.reserve(imageCount);
    framebuffers.reserve(imageCount);
    perImageFences.resize(imageCount);

    auto swapchainImageViews = swapchain.getImageViews();

    renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
        .device = device,
    };

    renderer::FenceCreateInfo fenceCreateInfo = {
        .device = device,
        .createFlags = renderer::FenceCreateFlags::START_SIGNALED,
    };

    for (std::uint32_t i = 0; i < imageCount; i++) {
        renderer::FramebufferCreateInfo framebufferCreateInfo = {
            .device = device,
            .renderPass = renderPass,
            .imageViews = {swapchainImageViews[i]},
        };

        framebuffers.emplace_back(framebufferCreateInfo);
        submitSemaphores.emplace_back(semaphoreCreateInfo);
    }

    for (std::uint32_t i = 0; i < frameCount; i++) {
        acquireSemaphores.emplace_back(semaphoreCreateInfo);
        fences.emplace_back(fenceCreateInfo);
    }

    bool running = true;

    data::ColourRGBA attachmentClearColour = {
        .r = 0.19f,
        .g = 0.37f,
        .b = 0.74f,
        .a = 1.0f,
    };

    while (running) {
        context.pollEvents();

        while (window.hasEvents()) {
            app::WindowEvent event = window.getNextEvent();

            switch (event.type) {
                case app::WindowEventType::CLOSED:
                    running = false;
                    break;

                default:
                    break;
            }
        }

        auto& fence = fences[frameIndex];
        auto& acquireSemaphore = acquireSemaphores[frameIndex];
        auto& commandBuffer = commandBuffers[frameIndex];

        device.waitForFences({fence});
        device.resetFences({fence});

        std::uint32_t imageIndex = 0;

        bool resized = false;

        while (true) {
            if (swapchain.shouldRecreate()) {
                device.waitIdle();

                renderer::SwapchainRecreateInfo swapchainRecreateInfo = {
                    .imageCount = imageCount,
                    .synchronise = synchronise,
                };

                swapchain.recreate(swapchainRecreateInfo);

                resized = true;
            }

            imageIndex = swapchain.acquireNextImage(acquireSemaphore);

            if (!swapchain.shouldRecreate()) {
                break;
            }
        }

        if (resized) {
            swapchainImageViews = swapchain.getImageViews();

            framebuffers.clear();
            submitSemaphores.clear();

            imageCount = swapchain.getImageCount();

            perImageFences.resize(imageCount);

            for (std::uint32_t i = 0; i < imageCount; i++) {
                renderer::FramebufferCreateInfo framebufferCreateInfo = {
                    .device = device,
                    .renderPass = renderPass,
                    .imageViews = {swapchainImageViews[i]},
                };

                framebuffers.emplace_back(framebufferCreateInfo);
                submitSemaphores.emplace_back(semaphoreCreateInfo);
            }
        }

        auto& framebuffer = framebuffers[imageIndex];
        auto& submitSemaphore = submitSemaphores[imageIndex];

        renderer::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .renderArea = {
                .offset = {0, 0},
                .extent = swapchain.getExtent(),
            },
            .clearValues = {
                attachmentClearColour,
            },
            .depthClearValue = {},
            .stencilClearValue = {},
        };

        commandBuffer.beginCapture();
        commandBuffer.beginRenderPass(renderPassBeginInfo);

        commandBuffer.endRenderPass();
        commandBuffer.endCapture();

        renderer::SubmitInfo submitInfo = {
            .fence = {fence},
            .commandBuffers = {commandBuffer},
            .waits = {acquireSemaphore},
            .signals = {submitSemaphore},
            .waitFlags = {renderer::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT},
        };

        renderQueue.submit(submitInfo);

        swapchain.presentNextImage(submitSemaphore);

        frameIndex = (frameIndex + 1) % frameCount;
    }

    device.waitIdle();
}

int main() {
    try {
        run();

        return 0;
    }
    catch (const std::exception& exception) {
        std::println("Runtime exception: {}", exception.what());

        return 1;
    }
}