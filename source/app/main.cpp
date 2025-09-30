#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/resources/fence.hpp>
#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/semaphore.hpp>

#include <chrono>
#include <cmath>
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
        .applicationVersion = data::Version(0, 0, 1),
        .engineVersion = data::Version(0, 0, 1),
        .applicationName = window.getTitle(),
        .engineName = "Engine",
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
                .type = renderer::QueueType::RENDER,
                .surface = nullptr,
            },
            renderer::QueueCreateInfo{
                .type = renderer::QueueType::PRESENT,
                .surface = surface,
            },
        },
    };

    renderer::Device device(deviceCreateInfo);

    std::span<renderer::Queue> queues = device.getQueues();

    renderer::Queue& renderQueue = queues[0];
    renderer::Queue& presentQueue = queues[1];

    renderer::SwapchainCreateInfo swapchainCreateInfo = {
        .instance = instance,
        .surface = surface,
        .device = device,
        .presentQueue = presentQueue,
        .renderQueue = renderQueue,
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
        .level = renderer::CommandBufferLevel::PRIMARY,
        .count = frameCount,
    };

    std::vector<renderer::CommandBuffer> commandBuffers = commandPool.allocateCommandBuffers(commandBufferCreateInfo);

    renderer::RenderPassCreateInfo renderPassCreateInfo = {
        .device = device,
        .colourAttachments = {
            renderer::FrameAttachmentInfo{
                .format = swapchain.getImageFormat(),
                .loadOperation = renderer::LoadOperation::CLEAR,
                .storeOperation = renderer::StoreOperation::STORE,
                .sampleCount = 1,
            },
        },
        .depthAttachment = {},
        .stencilAttachment = {},
    };

    renderer::RenderPass renderPass(renderPassCreateInfo);

    std::vector<renderer::Semaphore> acquireSemaphores;
    std::vector<renderer::Semaphore> submitSemaphores;
    std::vector<renderer::Fence> fences;

    std::vector<renderer::Framebuffer> framebuffers;
    std::vector<data::NullableReference<renderer::Fence>> perImageFences;

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

    float time = 0.0;

    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCounter = 0;

    while (running) {
        frameCounter++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(currentTime - lastTime).count();

        if (elapsed >= 1.0f) {
            std::println("FPS: {}", frameCounter);
            frameCounter = 0;
            lastTime = currentTime;
        }

        data::ColourRGBA attachmentClearColour = {
            .r = -std::sin(time),
            .g = std::cos(time),
            .b = std::sin(time),
            .a = 1.0,
        };

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

        auto capture = commandBuffer.beginCapture();
        auto render = capture.beginRenderPass(renderPassBeginInfo);

        render.end();
        capture.end();

        renderQueue.submit({commandBuffer}, {acquireSemaphore}, {submitSemaphore}, {fence});

        swapchain.presentNextImage(submitSemaphore);

        frameIndex = (frameIndex + 1) % frameCount;

        time += 0.005f;
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