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

    void Queue::submit(const std::vector<data::Reference<CommandBuffer>>& commandBuffers, const std::vector<data::Reference<Semaphore>>& available, const std::vector<data::Reference<Semaphore>>& finished, Fence& inFlight) {
        if (commandBuffers.size() != available.size() || commandBuffers.size() != finished.size()) {
            throw std::runtime_error("Error calling renderer::Queue::submit(): Mismatch between command buffer count and sync object count");
        }

        std::vector<VkCommandBuffer> buffers(commandBuffers.size());
        std::vector<VkSemaphore> waits(available.size());
        std::vector<VkSemaphore> signals(finished.size());
        std::vector<VkPipelineStageFlags> flags(commandBuffers.size());

        for (std::size_t i = 0; i < commandBuffers.size(); i++) {
            buffers[i] = commandBuffers[i]->getVkCommandBuffer();
            waits[i] = available[i]->getVkSemaphore();
            signals[i] = finished[i]->getVkSemaphore();
            flags[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        VkSubmitInfo submitInfo = {
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

        if (vkQueueSubmit(queue_, static_cast<std::uint32_t>(commandBuffers.size()), &submitInfo, inFlight.getVkFence()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Queue::submit(): Failed to submit command buffers to queue");
        }
    }

    QueueType Queue::getType() const {
        return type_;
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