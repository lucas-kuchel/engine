#include <engine/engine.hpp>
#include <engine/staging_manager.hpp>

engine::StagingManager::StagingManager(Engine& engine)
    : engine_(engine) {
}

engine::StagingManager::~StagingManager() {
    deallocate();
}

void engine::StagingManager::rotate() {
    currentIndex_++;
    currentOffset_ = 0;
    if (currentIndex_ <= buffers_.size()) {
        currentIndex_ = 0;
    }
}

void engine::StagingManager::allocate(std::size_t count, std::size_t individualSize) {
    deallocate();

    buffers_.reserve(count);
    fences_.reserve(count);
    semaphores_.reserve(count);

    auto& renderer = engine_.getRenderer();
    auto& device = renderer.getDevice();

    vulkanite::renderer::BufferCreateInfo createInfo = {
        .device = device,
        .memoryType = vulkanite::renderer::MemoryType::HOST_VISIBLE,
        .usageFlags = vulkanite::renderer::BufferUsageFlags::TRANSFER_SOURCE,
        .sizeBytes = individualSize,
    };

    vulkanite::renderer::FenceCreateInfo fenceCreateInfo = {
        .device = device,
        .createFlags = vulkanite::renderer::FenceCreateFlags::START_SIGNALLED,
    };

    for (std::size_t i = 0; i < count; i++) {
        auto& buffer = buffers_.emplace_back();
        auto& fence = fences_.emplace_back();
        auto& semaphore = semaphores_.emplace_back();

        buffer.create(createInfo);
        fence.create(fenceCreateInfo);
        semaphore.create(device);
    }
}

void engine::StagingManager::deallocate() {
    for (std::size_t i = 0; i < buffers_.size(); i++) {
        buffers_[i].destroy();
        fences_[i].destroy();
        semaphores_[i].destroy();
    }

    buffers_.clear();
    fences_.clear();
    semaphores_.clear();
}

std::size_t& engine::StagingManager::getOffset() {
    return currentOffset_;
}

vulkanite::renderer::Buffer& engine::StagingManager::getCurrentBuffer() {
    return buffers_[currentIndex_];
}

vulkanite::renderer::Fence& engine::StagingManager::getCurrentFence() {
    return fences_[currentIndex_];
}

vulkanite::renderer::Semaphore& engine::StagingManager::getCurrentSemaphore() {
    return semaphores_[currentIndex_];
}