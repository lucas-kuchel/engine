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

    buffers_.resize(count);
    fences_.resize(count);
    semaphores_.resize(count);

    auto& renderer = engine_.getRenderer();
    auto& device = renderer.getDevice();

    renderer::BufferCreateInfo createInfo = {
        .device = device,
        .memoryType = renderer::MemoryType::HOST_VISIBLE,
        .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
        .sizeBytes = individualSize,
    };

    renderer::FenceCreateInfo fenceCreateInfo = {
        .device = device,
        .createFlags = renderer::FenceCreateFlags::START_SIGNALLED,
    };

    for (auto& buffer : buffers_) {
        buffer = renderer::Buffer::create(createInfo);
    }

    for (auto& fence : fences_) {
        fence = renderer::Fence::create(fenceCreateInfo);
    }

    for (auto& semaphore : semaphores_) {
        semaphore = renderer::Semaphore::create(device);
    }
}

void engine::StagingManager::deallocate() {
    for (auto& buffer : buffers_) {
        renderer::Buffer::destroy(buffer);
    }

    for (auto& fence : fences_) {
        renderer::Fence::destroy(fence);
    }

    for (auto& semaphore : semaphores_) {
        renderer::Semaphore::destroy(semaphore);
    }

    buffers_.clear();
    fences_.clear();
    semaphores_.clear();
}

std::size_t& engine::StagingManager::getOffset() {
    return currentOffset_;
}

renderer::Buffer& engine::StagingManager::getCurrentBuffer() {
    return buffers_[currentIndex_];
}

renderer::Fence& engine::StagingManager::getCurrentFence() {
    return fences_[currentIndex_];
}

renderer::Semaphore& engine::StagingManager::getCurrentSemaphore() {
    return semaphores_[currentIndex_];
}