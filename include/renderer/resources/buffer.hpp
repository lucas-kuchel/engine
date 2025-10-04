#pragma once

#include <renderer/resources/config.hpp>

#include <data/references.hpp>

#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class Buffer;

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

    class Buffer {
    public:
        Buffer(const BufferCreateInfo& createInfo);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = default;

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept = default;

        [[nodiscard]] std::span<std::uint8_t> map(std::uint64_t sizeBytes = VK_WHOLE_SIZE, std::uint64_t offsetBytes = 0);

        void unmap();

        [[nodiscard]] bool mapped() const;
        [[nodiscard]] bool mappable() const;

        [[nodiscard]] std::uint64_t size() const;

        [[nodiscard]] VkBuffer& getVkBuffer();
        [[nodiscard]] VmaAllocation& getVmaAllocation();

        [[nodiscard]] const VkBuffer& getVkBuffer() const;
        [[nodiscard]] const VmaAllocation& getVmaAllocation() const;

    private:
        VkBuffer buffer_ = VK_NULL_HANDLE;
        VmaAllocation memory_ = VK_NULL_HANDLE;

        bool hostCoherent_ = false;
        bool hostVisible_ = false;
        bool isMapped_ = false;

        data::Ref<Device> device_;

        std::uint64_t size_ = 0;
        std::uint64_t mapSize_ = 0;
        std::uint64_t mapOffset_ = 0;
    };
}