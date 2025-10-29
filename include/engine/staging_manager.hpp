#pragma once

#include <renderer/buffer.hpp>
#include <renderer/fence.hpp>
#include <renderer/semaphore.hpp>

#include <vector>

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

        renderer::Buffer& getCurrentBuffer();
        renderer::Fence& getCurrentFence();
        renderer::Semaphore& getCurrentSemaphore();

    private:
        std::vector<renderer::Buffer> buffers_;
        std::vector<renderer::Fence> fences_;
        std::vector<renderer::Semaphore> semaphores_;

        Engine& engine_;

        std::size_t currentIndex_ = 0;
        std::size_t currentOffset_ = 0;
    };
}