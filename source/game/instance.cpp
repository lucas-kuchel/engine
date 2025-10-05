#include <game/instance.hpp>

#include <app/program.hpp>

#include <cstring>
#include <fstream>

#include <stb_image.h>

namespace game {
    struct Vertex {
        data::Position2D<float> position;
        data::ColourRGB colour;
        data::Position2D<float> texCoord;
    };

    struct Triangle {
        std::uint32_t a = 0;
        std::uint32_t b = 0;
        std::uint32_t c = 0;
    };

    Instance::Instance(app::Program& program)
        : program_(program) {
    }

    Instance::~Instance() {
    }

    renderer::RenderPassCreateInfo Instance::makeRequiredRenderPass() {
        return {
            .device = program_.device(),
            .depthStencilAttachments = {},
            .colourAttachments = {
                renderer::ColourAttachmentInfo{
                    .format = program_.swapchain().format(),
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
    }

    void Instance::start() {
        auto& device = program_.device();
        auto& renderPass = program_.renderPass();
        auto& transferCommandPool = program_.transferCommandPool();
        auto& transferQueue = program_.transferQueue();

        renderer::CommandBufferCreateInfo transferCommandBuffersCreateInfo = {
            .count = 1,
        };

        std::vector<renderer::CommandBuffer> transferCommandBuffers = transferCommandPool.allocateCommandBuffers(transferCommandBuffersCreateInfo);

        auto& transferCommandBuffer = transferCommandBuffers.front();

        std::int32_t width;
        std::int32_t height;
        std::int32_t channels;

        std::uint8_t* pixels = stbi_load("assets/images/rainbow.png", &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load image");
        }

        renderer::ImageCreateInfo imageCreateInfo = {
            .device = device,
            .type = renderer::ImageType::IMAGE_2D,
            .format = renderer::ImageFormat::R8G8B8A8_UNORM,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::ImageUsageFlags::TRANSFER_DESTINATION | renderer::ImageUsageFlags::SAMPLED,
            .extent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
            .sampleCount = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
        };

        image_ = data::makeUnique<renderer::Image>(imageCreateInfo);

        renderer::BufferCreateInfo imageStagingBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = image_->size(),
        };

        renderer::Buffer imageStagingBuffer(imageStagingBufferCreateInfo);

        std::span<std::uint8_t> imageStagingBufferData = imageStagingBuffer.map(imageStagingBuffer.size(), 0);

        std::memcpy(imageStagingBufferData.data(), pixels, imageStagingBuffer.size());

        imageStagingBuffer.unmap();

        transferCommandBuffer.beginCapture();

        renderer::ImageMemoryBarrier imageBarrier1 = {
            .image = image_.ref(),
            .sourceQueue = nullptr,
            .destinationQueue = nullptr,
            .oldLayout = renderer::ImageLayout::UNDEFINED,
            .newLayout = renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspectMask = renderer::ImageAspectFlags::COLOUR,
            .sourceAccessFlags = renderer::AccessFlags::NONE,
            .destinationAccessFlags = renderer::AccessFlags::TRANSFER_WRITE,
        };

        transferCommandBuffer.pipelineBarrier(renderer::PipelineStageFlags::TOP_OF_PIPE, renderer::PipelineStageFlags::TRANSFER, {imageBarrier1});

        renderer::BufferImageCopyRegion bufferImageCopyRegion = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .imageAspectMask = renderer::ImageAspectFlags::COLOUR,
            .imageOffset = {0, 0, 0},
            .imageExtent = image_->extent(),
        };

        transferCommandBuffer.copyBufferToImage(imageStagingBuffer, image_.ref(), renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {bufferImageCopyRegion});

        renderer::ImageMemoryBarrier imageBarrier2 = {
            .image = image_.ref(),
            .sourceQueue = nullptr,
            .destinationQueue = nullptr,
            .oldLayout = renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
            .newLayout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspectMask = renderer::ImageAspectFlags::COLOUR,
            .sourceAccessFlags = renderer::AccessFlags::TRANSFER_WRITE,
            .destinationAccessFlags = renderer::AccessFlags::SHADER_READ,
        };

        transferCommandBuffer.pipelineBarrier(renderer::PipelineStageFlags::TRANSFER, renderer::PipelineStageFlags::FRAGMENT_SHADER, {imageBarrier2});

        renderer::ImageViewCreateInfo imageViewCreateInfo = {
            .image = image_.ref(),
            .type = renderer::ImageViewType::IMAGE_2D,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        imageView_ = data::makeUnique<renderer::ImageView>(imageViewCreateInfo);

        renderer::SamplerCreateInfo samplerCreateInfo = {
            .device = device,
            .minFilter = renderer::Filter::NEAREST,
            .magFilter = renderer::Filter::NEAREST,
            .mipmapMode = renderer::MipmapMode::NEAREST,
            .addressModeU = renderer::AddressMode::REPEAT,
            .addressModeV = renderer::AddressMode::REPEAT,
            .addressModeW = renderer::AddressMode::REPEAT,
            .borderColour = renderer::BorderColour::FLOAT_OPAQUE_BLACK,
            .enableAnisotropy = false,
            .maxAnisotropy = 0.0f,
            .enableCompare = false,
            .unnormalisedCoordinates = false,
            .comparison = renderer::CompareOperation::ALWAYS,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 1.0f,
        };

        sampler_ = data::makeUnique<renderer::Sampler>(samplerCreateInfo);

        std::ifstream vertexShader("assets/shaders/basic.vert.spv", std::ios::binary | std::ios::ate);
        std::ifstream fragmentShader("assets/shaders/basic.frag.spv", std::ios::binary | std::ios::ate);

        std::uint64_t vertexShaderSize = static_cast<std::uint64_t>(vertexShader.tellg());
        std::uint64_t fragmentShaderSize = static_cast<std::uint64_t>(fragmentShader.tellg());

        vertexShader.seekg(0, std::ios::beg);
        fragmentShader.seekg(0, std::ios::beg);

        std::vector<std::uint32_t> vertexShaderBinary(vertexShaderSize / sizeof(std::uint32_t));
        std::vector<std::uint32_t> fragmentShaderBinary(fragmentShaderSize / sizeof(std::uint32_t));

        vertexShader.read(reinterpret_cast<char*>(vertexShaderBinary.data()), static_cast<std::uint32_t>(vertexShaderSize));
        fragmentShader.read(reinterpret_cast<char*>(fragmentShaderBinary.data()), static_cast<std::uint32_t>(fragmentShaderSize));

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

        renderer::DescriptorSetInputInfo inputInfo = {
            .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .stageFlags = renderer::DescriptorShaderStageFlags::FRAGMENT,
            .count = 1,
            .binding = 0,
        };

        renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .device = device,
            .inputs = {inputInfo},
        };

        descriptorSetLayout_ = data::makeUnique<renderer::DescriptorSetLayout>(layoutCreateInfo);

        renderer::DescriptorPoolSize size = {
            .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .count = 1,
        };

        renderer::DescriptorPoolCreateInfo poolCreateInfo = {
            .device = device,
            .poolSizes = {size},
            .maximumSetCount = 1,
        };

        descriptorPool_ = data::makeUnique<renderer::DescriptorPool>(poolCreateInfo);

        renderer::DescriptorSetCreateInfo setCreateInfo = {
            .layouts = {descriptorSetLayout_.ref()},
        };

        descriptorSets_ = descriptorPool_->allocateDescriptorSets(setCreateInfo);
        descriptorSet_ = descriptorSets_.front();

        renderer::DescriptorSetImageBinding imageBinding = {
            .image = imageView_.ref(),
            .sampler = sampler_.ref(),
            .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        };

        renderer::DescriptorSetUpdateInfo setUpdateInfo = {
            .set = descriptorSet_.get(),
            .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .binding = 0,
            .arrayElement = 0,
            .buffers = {},
            .images = {imageBinding},
        };

        descriptorPool_->updateDescriptorSets({setUpdateInfo});

        renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .device = device,
            .inputLayouts = {descriptorSetLayout_.ref()},
            .pushConstants = {},
        };

        basicPipelineLayout_ = data::makeUnique<renderer::PipelineLayout>(pipelineLayoutCreateInfo);

        renderer::PipelineCreateInfo pipelineCreateInfo = {
            .renderPass = renderPass,
            .layout = basicPipelineLayout_.ref(),
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
                        .inputRate = renderer::VertexInputRate::PER_VERTEX,
                        .binding = 0,
                        .strideBytes = sizeof(Vertex),
                    },
                },
                .attributes = {
                    renderer::VertexAttributeDescription{
                        .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                        .binding = 0,
                        .location = 0,

                    },
                    renderer::VertexAttributeDescription{
                        .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                        .binding = 0,
                        .location = 1,
                    },
                    renderer::VertexAttributeDescription{
                        .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                        .binding = 0,
                        .location = 2,
                    },
                },
            },
            .inputAssembly = {
                .topology = renderer::PolygonTopology::TRIANGLE_STRIP,
                .primitiveRestart = false,
            },
            .viewportCount = 1,
            .scissorCount = 1,
            .rasterisation = {},
            .multisample = {},
            .colourBlend = {
                .attachments = {renderer::ColourBlendAttachment{}},
            },
        };

        pipelines_ = device.createPipelines({pipelineCreateInfo});

        basicPipeline_ = pipelines_.front();

        std::array<Vertex, 4> vertices = {
            Vertex({0.5, -0.5}, {1.0, 0.5, 0.0}, {0.0, 0.0}),
            Vertex({-0.5, -0.5}, {0.5, 1.0, 0.5}, {1.0, 0.0}),
            Vertex({-0.5, 0.5}, {0.0, 0.5, 1.0}, {1.0, 1.0}),
            Vertex({0.5, 0.5}, {0.0, 0.0, 0.5}, {0.0, 1.0}),
        };

        std::array<Triangle, 2> triangles = {
            Triangle{0, 1, 2},
            Triangle{0, 2, 3},
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = vertices.size() * sizeof(Vertex),
        };

        renderer::BufferCreateInfo indexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::INDEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = triangles.size() * sizeof(Triangle),

        };

        vertexBuffer_ = data::makeUnique<renderer::Buffer>(vertexBufferCreateInfo);
        indexBuffer_ = data::makeUnique<renderer::Buffer>(indexBufferCreateInfo);

        renderer::BufferCreateInfo vertexStagingBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = vertices.size() * sizeof(Vertex),
        };

        renderer::BufferCreateInfo indexStagingBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = triangles.size() * sizeof(Triangle),
        };

        renderer::Buffer vertexStagingBuffer(vertexStagingBufferCreateInfo);
        renderer::Buffer indexStagingBuffer(indexStagingBufferCreateInfo);

        auto vertexStagingBufferMapping = vertexStagingBuffer.map(vertexStagingBuffer.size(), 0);
        auto indexStagingBufferMapping = indexStagingBuffer.map(indexStagingBuffer.size(), 0);

        std::memcpy(vertexStagingBufferMapping.data(), vertices.data(), vertexStagingBuffer.size());
        std::memcpy(indexStagingBufferMapping.data(), triangles.data(), indexStagingBuffer.size());

        vertexStagingBuffer.unmap();
        indexStagingBuffer.unmap();

        renderer::BufferCopyRegion vertexCopyRegion = {
            .sourceOffsetBytes = 0,
            .destinationOffsetBytes = 0,
            .sizeBytes = vertexStagingBuffer.size(),
        };

        renderer::BufferCopyRegion indexCopyRegion = {
            .sourceOffsetBytes = 0,
            .destinationOffsetBytes = 0,
            .sizeBytes = indexStagingBuffer.size(),
        };

        transferCommandBuffer.copyBuffer(vertexStagingBuffer, vertexBuffer_.ref(), {vertexCopyRegion});
        transferCommandBuffer.copyBuffer(indexStagingBuffer, indexBuffer_.ref(), {indexCopyRegion});

        transferCommandBuffer.endCapture();

        renderer::Fence fence({device, false});

        renderer::SubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        transferQueue.submit(submitInfo);

        device.waitForFences({fence});
    }

    void Instance::update() {
    }

    void Instance::close() {
    }

    void Instance::render() {
        auto& renderPass = program_.renderPass();
        auto& graphicsQueue = program_.graphicsQueue();
        auto& swapchain = program_.swapchain();

        auto& commandBuffer = program_.currentCommandBuffer();
        auto& framebuffer = program_.currentFramebuffer();
        auto& acquireSemaphore = program_.currentAcquireSemaphore();
        auto& presentSemaphore = program_.currentPresentSemaphore();
        auto& inFlightFence = program_.currentFence();

        commandBuffer.beginCapture();

        data::Rect2D<std::int32_t, std::uint32_t> renderArea = {
            .offset = {0, 0},
            .extent = program_.swapchain().extent(),
        };

        data::ColourRGBA clearColour = {
            .r = 0.0,
            .g = 0.0,
            .b = 0.0,
            .a = 1.0,
        };

        renderer::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .renderArea = renderArea,
            .clearValues = {
                clearColour,
            },
            .depthClearValue = {},
            .stencilClearValue = {},
        };

        commandBuffer.beginRenderPass(renderPassBeginInfo);

        renderer::Viewport viewport = {
            .position = {
                .x = 0.0,
                .y = 0.0,
            },
            .extent = {
                .width = static_cast<float>(swapchain.extent().width),
                .height = static_cast<float>(swapchain.extent().height),
            },
            .depth = {
                .min = 0.0,
                .max = 1.0,
            },
        };

        renderer::Scissor scissor = {
            .offset = {0, 0},
            .extent = swapchain.extent(),
        };

        commandBuffer.bindPipeline(basicPipeline_.get());

        commandBuffer.setPipelineViewports({viewport}, 0);
        commandBuffer.setPipelineScissors({scissor}, 0);

        commandBuffer.bindDescriptorSets(renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_.ref(), 0, {descriptorSet_.get()});

        commandBuffer.bindVertexBuffers({vertexBuffer_.ref()}, {0}, 0);
        commandBuffer.bindIndexBuffer(indexBuffer_.ref(), 0, renderer::IndexType::UINT32);

        commandBuffer.drawIndexed(static_cast<std::uint32_t>(indexBuffer_->size() / sizeof(std::uint32_t)), 1, 0, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.endCapture();

        renderer::SubmitInfo submitInfo = {
            .fence = inFlightFence,
            .commandBuffers = {commandBuffer},
            .waits = {acquireSemaphore},
            .signals = {presentSemaphore},
            .waitFlags = {
                renderer::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
            },
        };

        graphicsQueue.submit(submitInfo);
    }
}