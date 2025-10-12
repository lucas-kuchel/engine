#include <renderer/command_buffer.hpp>
#include <renderer/fence.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/semaphore.hpp>
#include <renderer/surface.hpp>

namespace renderer {
    bool Queue::submit(Queue& queue, const QueueSubmitInfo& submitInfo) {
        std::vector<VkCommandBuffer> buffers(submitInfo.commandBuffers.size());
        std::vector<VkSemaphore> waits(submitInfo.waits.size());
        std::vector<VkSemaphore> signals(submitInfo.signals.size());
        std::vector<VkPipelineStageFlags> flags(submitInfo.waitFlags.size());

        for (std::size_t i = 0; i < buffers.size(); i++) {
            buffers[i] = submitInfo.commandBuffers[i].commandBuffer_;
        }

        for (std::size_t i = 0; i < waits.size(); i++) {
            flags[i] = PipelineStageFlags::mapFrom(submitInfo.waitFlags[i]);
            waits[i] = submitInfo.waits[i].semaphore_;
        }

        for (std::size_t i = 0; i < signals.size(); i++) {
            signals[i] = submitInfo.signals[i].semaphore_;
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

        return vkQueueSubmit(queue.queue_, static_cast<std::uint32_t>(buffers.size()), &vkSubmitInfo, (submitInfo.fence.fence_ != nullptr) ? submitInfo.fence.fence_ : VK_NULL_HANDLE) == VK_SUCCESS;
    }
}