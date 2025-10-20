#pragma once

#include <renderer/configuration.hpp>

#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    struct BufferCopyRegion {
        std::uint64_t sourceOffsetBytes;
        std::uint64_t destinationOffsetBytes;
        std::uint64_t sizeBytes;
    };

    struct BufferCreateInfo {
        Device& device;
        MemoryType memoryType;
        Flags usageFlags;

        std::uint64_t sizeBytes;
    };

    struct BufferMapping {
        std::span<std::uint8_t> data;

        std::uint64_t offset = 0;

        std::uint64_t alignedSize = 0;
        std::uint64_t alignedOffset = 0;
    };

    class Buffer {
    public:
        static Buffer create(const BufferCreateInfo& createInfo);
        static void destroy(Buffer& buffer);

        static BufferMapping map(Buffer& buffer, std::uint64_t size, std::uint64_t offset);
        static void unmap(Buffer& buffer, BufferMapping& mapping);

        static std::uint64_t size(Buffer& buffer);
        static bool mappable(Buffer& buffer);

        explicit operator bool() {
            return buffer_ && allocation_ && device_;
        }

    private:
        VkBuffer buffer_ = nullptr;
        VmaAllocation allocation_ = nullptr;
        Device* device_ = nullptr;

        bool isHostCoherent_ = false;
        bool isHostVisible_ = false;

        std::uint64_t size_ = 0;

        friend class CommandBuffer;
        friend class DescriptorPool;
    };
}