#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>

#include <stdexcept>

namespace renderer {
    Queue::Queue() {
    }

    Queue::~Queue() {
    }

    void Queue::create(const QueueCreateInfo& createInfo) {
        auto& instanceData = createInfo.instance.getData();
        auto& surfaceData = createInfo.surface.getData();

        VkQueueFlagBits queueTypeNeeded = VK_QUEUE_FLAG_BITS_MAX_ENUM;
        bool isPresentType = false;

        switch (createInfo.type) {
            case QueueType::COMPUTE:
                queueTypeNeeded = VK_QUEUE_COMPUTE_BIT;
                break;

            case QueueType::RENDER:
                queueTypeNeeded = VK_QUEUE_GRAPHICS_BIT;
                break;

            case QueueType::TRANSFER:
                queueTypeNeeded = VK_QUEUE_TRANSFER_BIT;
                break;

            case QueueType::PRESENT:
                isPresentType = true;
                break;
        }

        for (std::uint32_t i = 0; i < instanceData.queueFamilyProperties.size(); i++) {
            const auto& family = instanceData.queueFamilyProperties[i];

            if (isPresentType) {
                VkBool32 presentSupported = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(instanceData.physicalDevice, i, surfaceData.surface, &presentSupported);

                if (presentSupported) {
                    data_.familyIndex = i;

                    if (instanceData.queueFamilyOccupations[i] == family.queueCount) {
                        instanceData.queueFamilyOccupations[i] = 0;
                    }

                    data_.queueIndex = instanceData.queueFamilyOccupations[i];
                    instanceData.queueFamilyOccupations[i]++;

                    break;
                }
            }
            else if (family.queueFlags & queueTypeNeeded) {
                data_.familyIndex = i;

                if (instanceData.queueFamilyOccupations[i] == family.queueCount) {
                    instanceData.queueFamilyOccupations[i] = 0;
                }

                data_.queueIndex = instanceData.queueFamilyOccupations[i];
                instanceData.queueFamilyOccupations[i]++;

                break;
            }
        }

        if (data_.familyIndex == std::numeric_limits<std::uint32_t>::max()) {
            throw std::runtime_error("Error calling renderer::Queue::create(): Failed to find queue family for requested queue");
        }
    }

    QueueData& Queue::getData() {
        return data_;
    }
}