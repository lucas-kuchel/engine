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

        struct FlagMap {
            uint32_t submitFlag;
            VkPipelineStageFlagBits vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {SubmitWaitFlags::TOP_OF_PIPE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {SubmitWaitFlags::DRAW_INDIRECT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT},
            {SubmitWaitFlags::VERTEX_INPUT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT},
            {SubmitWaitFlags::VERTEX_SHADER, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT},
            {SubmitWaitFlags::FRAGMENT_SHADER, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
            {SubmitWaitFlags::EARLY_FRAGMENT_TESTS, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
            {SubmitWaitFlags::LATE_FRAGMENT_TESTS, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT},
            {SubmitWaitFlags::COLOR_ATTACHMENT_OUTPUT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            {SubmitWaitFlags::TRANSFER, VK_PIPELINE_STAGE_TRANSFER_BIT},
            {SubmitWaitFlags::BOTTOM_OF_PIPE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT},
            {SubmitWaitFlags::HOST, VK_PIPELINE_STAGE_HOST_BIT},
            {SubmitWaitFlags::ALL_GRAPHICS, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT},
            {SubmitWaitFlags::ALL_COMMANDS, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT},
        };

        for (std::size_t i = 0; i < buffers.size(); i++) {
            buffers[i] = submitInfo.commandBuffers[i]->getVkCommandBuffer();
        }

        for (std::size_t i = 0; i < waits.size(); i++) {
            for (const auto& mapping : flagMapping) {
                if (submitInfo.waitFlags[i] & mapping.submitFlag) {
                    flags[i] |= static_cast<std::uint32_t>(mapping.vkFlag);
                }
            }

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