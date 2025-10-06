#include <game/player.hpp>
#include <game/settings.hpp>

#include <renderer/device.hpp>
#include <renderer/queue.hpp>

#include <renderer/resources/fence.hpp>
#include <renderer/resources/sampler.hpp>

#include <cstring>

#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

namespace game {
    Player::Player(SettingsConfig& settings, renderer::Device& device, renderer::CommandBuffer& transferCommandBuffer, renderer::Queue& transferCommandQueue)
        : settings_(settings) {
        transferCommandBuffer.beginCapture();

        std::array<Vertex, 4> vertices = {
            Vertex({0.5, -1.0}, {1.0, 0.0}),
            Vertex({-0.5, -1.0}, {0.0, 0.0}),
            Vertex({-0.5, 1.0}, {0.0, 2.0}),
            Vertex({0.5, 1.0}, {1.0, 2.0}),
        };

        std::array<Triangle, 2> triangles = {
            Triangle({0, 1, 2}),
            Triangle({0, 2, 3}),
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

        auto vertexStagingBufferData = vertexStagingBuffer.map();
        auto indexStagingBufferData = indexStagingBuffer.map();

        std::memcpy(vertexStagingBufferData.data(), vertices.data(), vertexStagingBuffer.size());
        std::memcpy(indexStagingBufferData.data(), triangles.data(), indexStagingBuffer.size());

        vertexStagingBuffer.unmap();
        indexStagingBuffer.unmap();

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = 0,
            .destinationOffsetBytes = 0,
            .sizeBytes = vertexStagingBuffer.size(),
        };

        renderer::BufferCopyRegion indexBufferCopyRegion = {
            .sourceOffsetBytes = 0,
            .destinationOffsetBytes = 0,
            .sizeBytes = indexStagingBuffer.size(),
        };

        transferCommandBuffer.copyBuffer(vertexStagingBuffer, vertexBuffer_.ref(), {vertexBufferCopyRegion});
        transferCommandBuffer.copyBuffer(indexStagingBuffer, indexBuffer_.ref(), {indexBufferCopyRegion});

        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;

        std::string path = "assets/images/indicator.png";

        std::uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        std::uint64_t size = static_cast<std::uint64_t>(width * height * channels);

        if (!pixels) {
            throw std::runtime_error(std::format("Failed to load image: {}", path));
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
            .sizeBytes = size,
        };

        renderer::Buffer imageStagingBuffer(imageStagingBufferCreateInfo);

        std::span<std::uint8_t> imageStagingBufferData = imageStagingBuffer.map(imageStagingBuffer.size(), 0);

        std::memcpy(imageStagingBufferData.data(), pixels, imageStagingBuffer.size());

        imageStagingBuffer.unmap();

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

        transferCommandBuffer.endCapture();

        renderer::Fence fence({device, 0});

        renderer::SubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        transferCommandQueue.submit(submitInfo);

        device.waitForFences({fence});

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

        renderer::DescriptorSetInputInfo imageInputInfo = {
            .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .stageFlags = renderer::DescriptorShaderStageFlags::FRAGMENT,
            .count = 1,
            .binding = 0,
        };

        renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .device = device,
            .inputs = {imageInputInfo},
        };

        descriptorSetLayout_ = data::makeUnique<renderer::DescriptorSetLayout>(layoutCreateInfo);

        renderer::DescriptorPoolSize imageSize = {
            .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .count = 1,
        };

        renderer::DescriptorPoolCreateInfo poolCreateInfo = {
            .device = device,
            .poolSizes = {imageSize},
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

        renderer::DescriptorSetUpdateInfo imageSetUpdateInfo = {
            .set = descriptorSet_.get(),
            .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .binding = 0,
            .arrayElement = 0,
            .buffers = {},
            .images = {imageBinding},
        };

        descriptorPool_->updateDescriptorSets({imageSetUpdateInfo});

        stbi_image_free(pixels);
    }

    Player::~Player() {
    }

    void Player::beginMove(const app::WindowKeyPressedEventInfo& pressEvent) {
        if (pressEvent.key == app::Key::A) {
            deltaX_ -= 1;
        }

        if (pressEvent.key == app::Key::D) {
            deltaX_ += 1;
        }
    }

    void Player::stopMove(const app::WindowKeyReleasedEventInfo& releaseEvent) {
        if (releaseEvent.key == app::Key::A) {
            deltaX_ += 1;
        }

        if (releaseEvent.key == app::Key::D) {
            deltaX_ -= 1;
        }
    }

    void Player::update(float deltaTime) {
        glm::vec2 direction = glm::vec2(static_cast<float>(deltaX_), 0.0f);

        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction);
            position_ += direction * settings_.controls.speed * deltaTime;

            orientation_ = direction;

            mirrorX_ = (deltaX_ < 0) ? -1.0f : 1.0f;
        }

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(mirrorX_, 1.0f, 1.0f));

        model_ = glm::mat4(1.0f);
        model_ = glm::translate(model_, glm::vec3(position_, 0.0f)) * scale;
    }

    void Player::render(renderer::CommandBuffer& graphicsCommandBuffer, renderer::PipelineLayout& layout) {
        std::span<std::uint8_t> serial = {reinterpret_cast<std::uint8_t*>(&model_), sizeof(glm::mat4)};

        graphicsCommandBuffer.pushConstants(layout, renderer::DescriptorShaderStageFlags::VERTEX, serial, 0);
        graphicsCommandBuffer.bindVertexBuffers({vertexBuffer_.ref()}, {0}, 0);
        graphicsCommandBuffer.bindIndexBuffer(indexBuffer_.ref(), 0, renderer::IndexType::UINT32);
        graphicsCommandBuffer.drawIndexed(static_cast<std::uint32_t>(indexBuffer_->size() / sizeof(std::uint32_t)), 1, 0, 0, 0);
    }

    renderer::DescriptorSet& Player::descriptorSet() {
        return descriptorSet_.get();
    }

    renderer::DescriptorSetLayout& Player::descriptorSetLayout() {
        return descriptorSetLayout_.ref();
    }

    renderer::PushConstantInputInfo Player::pushConstant() {
        return {
            .sizeBytes = sizeof(glm::mat4),
            .stageFlags = renderer::DescriptorShaderStageFlags::VERTEX,
        };
    }

    glm::vec2 Player::position() const {
        return position_;
    }

    glm::vec2 Player::orientation() const {
        return orientation_;
    }
}