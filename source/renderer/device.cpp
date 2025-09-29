#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>

#include <stdexcept>

namespace renderer {
    Device::Device(const DeviceCreateInfo& createInfo)
        : instance_(createInfo.instance) {
        auto& physicalDevice = instance_->getVkPhysicalDevice();

        std::vector<std::uint32_t> familyIndexMappings;
        std::vector<std::vector<float>> familyIndexPriorities;

        queues_.reserve(createInfo.queues.size());

        for (auto& queueCreateInfo : createInfo.queues) {
            queues_.push_back(Queue());

            auto& queue = queues_.back();

            VkQueueFlagBits queueTypeNeeded = VK_QUEUE_FLAG_BITS_MAX_ENUM;
            bool isPresentType = false;

            switch (queueCreateInfo.type) {
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

            auto& queueFamilyProperties = instance_->queueFamilyProperties_;
            auto& queueFamilyOccupations = instance_->queueFamilyOccupations_;
            auto& physicalDevice = instance_->physicalDevice_;

            for (std::uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
                const auto& family = queueFamilyProperties[i];

                if (isPresentType) {
                    VkBool32 presentSupported = VK_FALSE;

                    if (queueCreateInfo.surface == nullptr) {
                        throw std::runtime_error("Error constructing renderer::Queue inside renderer::Device: Present queues require a surface to be created");
                    }

                    auto& surface = queueCreateInfo.surface->getVkSurface();

                    if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupported) != VK_SUCCESS) {
                        throw std::runtime_error("Error constructing renderer::Queue inside renderer::Device: Failed to query surface presentation support");
                    }

                    if (presentSupported) {
                        queue.familyIndex_ = i;

                        if (queueFamilyOccupations[i] == family.queueCount) {
                            queueFamilyOccupations[i] = 0;
                        }

                        queue.queueIndex_ = queueFamilyOccupations[i];
                        queueFamilyOccupations[i]++;

                        break;
                    }
                }
                else if (family.queueFlags & queueTypeNeeded) {
                    queue.familyIndex_ = i;

                    if (queueFamilyOccupations[i] == family.queueCount) {
                        queueFamilyOccupations[i] = 0;
                    }

                    queue.queueIndex_ = queueFamilyOccupations[i];
                    queueFamilyOccupations[i]++;

                    break;
                }
            }

            if (queue.familyIndex_ == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error("Error constructing renderer::Queue inside renderer::Device: Failed to find queue family for requested queue");
            }

            if (familyIndexMappings.size() <= queue.familyIndex_) {
                familyIndexMappings.resize(queue.familyIndex_ + 1, 0);
                familyIndexPriorities.resize(queue.familyIndex_ + 1);
            }

            if (familyIndexMappings[queue.familyIndex_] < queueFamilyProperties[queue.familyIndex_].queueCount) {
                familyIndexMappings[queue.familyIndex_]++;
                familyIndexPriorities[queue.familyIndex_].push_back(1.0f);
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (std::uint32_t i = 0; i < familyIndexMappings.size(); i++) {
            if (familyIndexMappings[i] == 0 || familyIndexPriorities[i].size() == 0) {
                continue;
            }

            VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = i,
                .queueCount = familyIndexMappings[i],
                .pQueuePriorities = familyIndexPriorities[i].data(),
            };

            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::uint32_t extensionCount = 0;

        if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::create(): Failed to enumerate device extensions");
        }

        std::vector<VkExtensionProperties> extensionProperties(extensionCount);

        if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensionProperties.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::create(): Failed to enumerate device extensions");
        }

        std::vector<const char*> selectedExtensions;

        bool foundSwapchain = false;

        for (auto& extensionInfo : extensionProperties) {
            bool match = false;

            match |= std::string_view(extensionInfo.extensionName) == "VK_KHR_swapchain";

            if (match) {
                foundSwapchain = true;

                selectedExtensions.push_back(extensionInfo.extensionName);
            }
        }

        if (!foundSwapchain) {
            throw std::runtime_error("Error calling renderer::Device::create(): Swapchain is unsupported on this system");
        }

        std::uint32_t extensionInfoCount = selectedExtensions.size();
        std::uint32_t queueCreateInfoCount = queueCreateInfos.size();

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = queueCreateInfoCount,
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = extensionInfoCount,
            .ppEnabledExtensionNames = selectedExtensions.data(),
            .pEnabledFeatures = nullptr,
        };

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device_) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::create(): Failed to create device");
        }

        for (auto& queue : queues_) {
            vkGetDeviceQueue(device_, queue.familyIndex_, queue.queueIndex_, &queue.queue_);
        }
    }

    Device::~Device() {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDevice(device_, nullptr);

            device_ = VK_NULL_HANDLE;
        }
    }

    std::span<Queue> Device::getQueues() {
        return queues_;
    }

    std::span<const Queue> Device::getQueues() const {
        return queues_;
    }

    VkDevice& Device::getVkDevice() {
        return device_;
    }

    const VkDevice& Device::getVkDevice() const {
        return device_;
    }
}