#pragma once

#include <vulkanite/renderer/renderer.hpp>

namespace engine {
    class Engine;

    class StagingManager {
    public:
        StagingManager(Engine& engine);
        ~StagingManager();

        void rotate();
        void allocate(std::size_t count, std::size_t individualSize);
        void deallocate();

        std::size_t& getOffset();

        vulkanite::renderer::Buffer& getCurrentBuffer();
        vulkanite::renderer::Fence& getCurrentFence();
        vulkanite::renderer::Semaphore& getCurrentSemaphore();

    private:
        std::vector<vulkanite::renderer::Buffer> buffers_;
        std::vector<vulkanite::renderer::Fence> fences_;
        std::vector<vulkanite::renderer::Semaphore> semaphores_;

        Engine& engine_;

        std::size_t currentIndex_ = 0;
        std::size_t currentOffset_ = 0;
    };
}