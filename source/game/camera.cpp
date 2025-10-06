#include <game/camera.hpp>
#include <game/player.hpp>
#include <game/settings.hpp>

#include <renderer/resources/fence.hpp>

#include <renderer/device.hpp>
#include <renderer/queue.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    Camera::Camera(SettingsConfig& settings, renderer::Device& device, renderer::CommandBuffer& transferCommandBuffer, renderer::Queue& transferQueue, renderer::Swapchain& swapchain)
        : device_(device), transferCommandBuffer_(transferCommandBuffer), transferQueue_(transferQueue), swapchain_(swapchain), settings_(settings) {
        transferCommandBuffer_.beginCapture();

        std::array<glm::mat4, 2> matrices = {
            projection_,
            view_,
        };

        renderer::BufferCreateInfo cameraBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = matrices.size() * sizeof(glm::mat4),
        };

        cameraBuffer_ = data::makeUnique<renderer::Buffer>(cameraBufferCreateInfo);

        renderer::BufferCreateInfo cameraStagingBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = cameraBuffer_->size(),
        };

        renderer::Buffer cameraStagingBuffer(cameraStagingBufferCreateInfo);

        auto cameraStagingBufferData = cameraStagingBuffer.map();

        std::memcpy(cameraStagingBufferData.data(), matrices.data(), cameraStagingBuffer.size());

        cameraStagingBuffer.unmap();

        renderer::BufferCopyRegion cameraBufferCopyRegion = {
            .sourceOffsetBytes = 0,
            .destinationOffsetBytes = 0,
            .sizeBytes = cameraStagingBuffer.size(),
        };

        transferCommandBuffer.copyBuffer(cameraStagingBuffer, cameraBuffer_.ref(), {cameraBufferCopyRegion});

        renderer::DescriptorSetInputInfo bufferInputInfo = {
            .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .stageFlags = renderer::DescriptorShaderStageFlags::VERTEX,
            .count = 1,
            .binding = 0,
        };

        renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .device = device,
            .inputs = {bufferInputInfo},
        };

        descriptorSetLayout_ = data::makeUnique<renderer::DescriptorSetLayout>(layoutCreateInfo);

        renderer::DescriptorPoolSize bufferSize = {
            .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .count = 1,
        };

        renderer::DescriptorPoolCreateInfo poolCreateInfo = {
            .device = device,
            .poolSizes = {bufferSize},
            .maximumSetCount = 1,
        };

        descriptorPool_ = data::makeUnique<renderer::DescriptorPool>(poolCreateInfo);

        renderer::DescriptorSetCreateInfo setCreateInfo = {
            .layouts = {descriptorSetLayout_.ref()},
        };

        descriptorSets_ = descriptorPool_->allocateDescriptorSets(setCreateInfo);
        descriptorSet_ = descriptorSets_.front();

        renderer::DescriptorSetBufferBinding bufferBinding = {
            .buffer = cameraBuffer_.ref(),
            .offsetBytes = 0,
            .rangeBytes = cameraBuffer_->size(),
        };

        renderer::DescriptorSetUpdateInfo imageSetUpdateInfo = {
            .set = descriptorSet_.get(),
            .inputType = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .binding = 0,
            .arrayElement = 0,
            .buffers = {bufferBinding},
            .images = {},
        };

        descriptorPool_->updateDescriptorSets({imageSetUpdateInfo});

        transferCommandBuffer_.endCapture();

        renderer::Fence fence({device_, 0});

        renderer::SubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer_},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        transferQueue_.submit(submitInfo);

        device_.waitForFences({fence});
    }

    Camera::~Camera() {
    }

    void Camera::slowMoveToPlayer(float deltaTime, const Player& player) {
        glm::vec2 delta = player.position() - position_;
        position_ += delta * settings_.camera.ease * deltaTime;
    }

    void Camera::update() {
        transferCommandBuffer_.beginCapture();

        auto extent = swapchain_.extent();

        float minExtent = static_cast<float>(std::min(extent.width, extent.height));

        float halfSize = (screenSizeWorldUnits_ * 0.5f) / settings_.camera.zoom;

        float halfWidth = halfSize * (static_cast<float>(extent.width) / minExtent);
        float halfHeight = halfSize * (static_cast<float>(extent.height) / minExtent);

        float left = -halfWidth;
        float right = +halfWidth;
        float bottom = -halfHeight;
        float top = +halfHeight;

        projection_ = glm::ortho(left, right, bottom, top);

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(-position_, 0.0f));
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotation_, glm::vec3(0.0f, 0.0f, 1.0f));

        view_ = rotation * translation;

        std::array<glm::mat4, 2> matrices = {
            projection_,
            view_,
        };

        renderer::BufferCreateInfo cameraStagingBufferCreateInfo = {
            .device = device_,
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = cameraBuffer_->size(),
        };

        renderer::Buffer cameraStagingBuffer(cameraStagingBufferCreateInfo);

        auto cameraStagingBufferData = cameraStagingBuffer.map();

        std::memcpy(cameraStagingBufferData.data(), matrices.data(), cameraStagingBuffer.size());

        cameraStagingBuffer.unmap();

        renderer::BufferCopyRegion cameraBufferCopyRegion = {
            .sourceOffsetBytes = 0,
            .destinationOffsetBytes = 0,
            .sizeBytes = cameraStagingBuffer.size(),
        };

        transferCommandBuffer_.copyBuffer(cameraStagingBuffer, cameraBuffer_.ref(), {cameraBufferCopyRegion});

        transferCommandBuffer_.endCapture();

        renderer::Fence fence({device_, 0});

        renderer::SubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer_},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        transferQueue_.submit(submitInfo);

        device_.waitForFences({fence});
    }

    renderer::DescriptorSet& Camera::descriptorSet() {
        return descriptorSet_.get();
    }

    renderer::DescriptorSetLayout& Camera::descriptorSetLayout() {
        return descriptorSetLayout_.ref();
    }
}