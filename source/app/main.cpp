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

    filesystem::BinaryFile<std::uint32_t> vertexShaderFile("assets/shaders/basic.vert.spv");
    filesystem::BinaryFile<std::uint32_t> fragmentShaderFile("assets/shaders/basic.frag.spv");

    std::vector vertexShaderBinary = vertexShaderFile.read();
    std::vector fragmentShaderBinary = fragmentShaderFile.read();

    renderer::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {
        .device = device,
        .data = vertexShaderBinary,
    };

    renderer::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {
        .device = device,
        .data = fragmentShaderBinary,
    };

    renderer::ShaderModule vertexShaderModule(vertexShaderModuleCreateInfo);
    renderer::ShaderModule fragmentShaderModule(fragmentShaderModuleCreateInfo);

    renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .device = device,
        .inputLayouts = {},
        .pushConstants = {},
    };

    renderer::PipelineLayout pipelineLayout(pipelineLayoutCreateInfo);

    renderer::Viewport viewport = {
        {
            .x = 0.0,
            .y = 0.0,
        },
        {
            .width = static_cast<float>(window.getExtent().width),
            .height = static_cast<float>(window.getExtent().height),
        },
        {
            .min = 0.0,
            .max = 1.0,
        },
    };

    renderer::Scissor scissor = {
        {
            .x = 0,
            .y = 0,
        },
        window.getExtent(),
    };

    renderer::PipelineCreateInfo pipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = pipelineLayout,
        .subpassIndex = 0,
        .shaderStages = {
            {
                vertexShaderModule,
                renderer::ShaderStage::VERTEX,
                "main",
            },
            {
                fragmentShaderModule,
                renderer::ShaderStage::FRAGMENT,
                "main",
            },
        },
        .vertexInput = {
            .bindings = {
                renderer::VertexInputBindingDescription{
                    .binding = 0,
                    .strideBytes = sizeof(float[2]) + sizeof(float[3]),
                    .inputRate = renderer::VertexInputRate::PER_VERTEX,
                },
            },
            .attributes = {
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .location = 0,
                    .binding = 0,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .location = 1,
                    .binding = 0,
                },
            },
        },
        .inputAssembly = {
            renderer::RasterisationPrimitive::TRIANGLE,
            false,
        },
        .viewports = {
            viewport,
        },
        .scissors = {
            scissor,
        },
        .rasterisation = {
            .frontFaceWinding = renderer::PolygonFaceWinding::CLOCKWISE,
            .cullMode = renderer::PolygonCullMode::NONE,
            .frontface = {},
            .backface = {},
            .depthClampEnable = false,
            .depthTestEnable = false,
            .depthWriteEnable = false,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
        },
        .multisample = {
            .sampleCount = 1,
            .sampleShadingEnable = false,
            .alphaToCoverageEnable = false,
            .alphaToOneEnable = false,
            .minSampleShading = 0.0f,
        },
        .colourBlend = {
            .attachments = {
                {
                    .blendEnable = false,
                    .sourceColourBlendFactor = renderer::BlendFactor::ONE,
                    .destinationColourBlendFactor = renderer::BlendFactor::ZERO,
                    .colourBlendOperation = renderer::BlendOperation::ADD,
                    .sourceAlphaBlendFactor = renderer::BlendFactor::ONE,
                    .destinationAlphaBlendFactor = renderer::BlendFactor::ZERO,
                    .alphaBlendOperation = renderer::BlendOperation::ADD,
                },
            },
        },
    };

    std::vector<renderer::Pipeline> pipelines = device.createPipelines({pipelineCreateInfo});

    bool running = true;

    data::ColourRGBA attachmentClearColour = {
        .r = 0.64f,
        .g = 0.21f,
        .b = 0.47f,
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

        auto capture = commandBuffer.beginCapture();
        auto render = capture.beginRenderPass(renderPassBeginInfo);

        render.end();
        capture.end();

        renderQueue.submit({commandBuffer}, {acquireSemaphore}, {submitSemaphore}, {fence});

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