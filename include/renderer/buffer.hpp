#pragma once

#include <renderer/configuration.hpp>

#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    struct BufferCopyRegion {
        std::size_t sourceOffsetBytes;
        std::size_t destinationOffsetBytes;
        std::size_t sizeBytes;
    };

    struct BufferCreateInfo {
        Device& device;
        MemoryType memoryType;
        Flags usageFlags;

        std::size_t sizeBytes;
    };

    struct BufferMapping {
        std::span<std::uint8_t> data;

        std::size_t offset = 0;

        std::size_t alignedSize = 0;
        std::size_t alignedOffset = 0;
    };

    class Buffer {
    public:
        static Buffer create(const BufferCreateInfo& createInfo);
        static void destroy(Buffer& buffer);

        static BufferMapping map(Buffer& buffer, std::size_t size, std::size_t offset);
        static void unmap(Buffer& buffer, BufferMapping& mapping);

        static std::size_t size(Buffer& buffer);
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

        std::size_t size_ = 0;

        friend class CommandBuffer;
        friend class DescriptorPool;
    };
}