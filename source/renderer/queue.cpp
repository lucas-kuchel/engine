#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>

#include <renderer/commands/buffer.hpp>

#include <renderer/resources/fence.hpp>
#include <renderer/resources/semaphore.hpp>

#include <stdexcept>

namespace renderer {
    Queue::~Queue() {
    }

    void Queue::submit(const SubmitInfo& submitInfo) {
        std::vector<VkCommandBuffer> buffers(submitInfo.commandBuffers.size());
        std::vector<VkSemaphore> waits(submitInfo.waits.size());
        std::vector<VkSemaphore> signals(submitInfo.signals.size());
        std::vector<VkPipelineStageFlags> flags(submitInfo.waitFlags.size());

        for (std::size_t i = 0; i < buffers.size(); i++) {
            buffers[i] = submitInfo.commandBuffers[i]->getVkCommandBuffer();
        }

        for (std::size_t i = 0; i < waits.size(); i++) {
            flags[i] = PipelineStageFlags::mapFrom(submitInfo.waitFlags[i]);
            waits[i] = submitInfo.waits[i]->getVkSemaphore();
        }

        for (std::size_t i = 0; i < signals.size(); i++) {
            signals[i] = submitInfo.signals[i]->getVkSemaphore();
        }

        VkSubmitInfo vkSubmitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<std::uint32_t>(waits.size()),
            .pWaitSemaphores = waits.data(),
            .pWaitDstStageMask = flags.data(),
            .commandBufferCount = static_cast<std::uint32_t>(buffers.size()),
            .pCommandBuffers = buffers.data(),
            .signalSemaphoreCount = static_cast<std::uint32_t>(signals.size()),
            .pSignalSemaphores = signals.data(),
        };

        if (vkQueueSubmit(queue_, static_cast<std::uint32_t>(buffers.size()), &vkSubmitInfo, (submitInfo.fence != nullptr) ? submitInfo.fence->getVkFence() : VK_NULL_HANDLE)) {
            throw std::runtime_error("Call failed: renderer::Queue::submit(): Failed to submit command buffers to queue");
        }
    }

    VkQueue& Queue::getVkQueue() {
        return queue_;
    }

    const VkQueue& Queue::getVkQueue() const {
        return queue_;
    }

    std::uint32_t Queue::getVkFamilyIndex() const {
        return familyIndex_;
    }

    std::uint32_t Queue::getVkQueueIndex() const {
        return queueIndex_;
    }
}