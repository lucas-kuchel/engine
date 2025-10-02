#pragma once

#include <data/references.hpp>

#include <cstdint>
#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class Buffer;

    struct BufferUsageFlags {
        enum {
            VERTEX = 1 << 0,
            INDEX = 1 << 1,
            UNIFORM = 1 << 2,
            STORAGE = 1 << 3,
            TRANSFER_SOURCE = 1 << 4,
            TRANSFER_DESTINATION = 1 << 5,
        };
    };

    using BufferUsage = std::uint32_t;

    // @brief Describes how allocated memory can be accessed
    enum class MemoryType {
        // @brief Visible to the CPU; changes to data will not be applied until flushed
        HOST_VISIBLE,

        // @brief Not visible to the CPU; most efficient for commonly used memory on the device
        DEVICE_LOCAL,
    };

    struct BufferCopyRegion {
        std::uint64_t sourceOffset;
        std::uint64_t destinationOffset;
        std::uint64_t size;
    };

    struct BufferCreateInfo {
        Device& device;

        MemoryType type;

        std::uint32_t sizeBytes;

        BufferUsage usage;
    };

    class BufferMemoryMapping {
    public:
        ~BufferMemoryMapping();

        BufferMemoryMapping(const BufferMemoryMapping&) = delete;
        BufferMemoryMapping(BufferMemoryMapping&&) = default;

        BufferMemoryMapping& operator=(const BufferMemoryMapping&) = delete;
        BufferMemoryMapping& operator=(BufferMemoryMapping&&) = default;

        [[nodiscard]] std::span<std::uint32_t> get();

        void unmap();

        [[nodiscard]] bool isMapped() const;

    private:
        BufferMemoryMapping(Buffer& buffer);

        data::Reference<Buffer> buffer_;

        void* data_ = nullptr;

        friend class Buffer;
    };

    class Buffer {
    public:
        Buffer(const BufferCreateInfo& createInfo);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = default;

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept = default;

        [[nodiscard]] BufferMemoryMapping map(std::uint32_t sizeBytes, std::uint32_t offsetBytes);

        [[nodiscard]] VkBuffer& getVkBuffer();
        [[nodiscard]] const VkBuffer& getVkBuffer() const;

        [[nodiscard]] VmaAllocation& getVmaAllocation();
        [[nodiscard]] const VmaAllocation& getVmaAllocation() const;

        [[nodiscard]] bool isMapped() const;

        [[nodiscard]] bool isMappable() const;

        [[nodiscard]] std::uint32_t getSize() const;

    private:
        VkBuffer buffer_ = VK_NULL_HANDLE;
        VmaAllocation memory_ = VK_NULL_HANDLE;

        bool hostCoherent_ = false;
        bool hostVisible_ = false;
        bool isMapped_ = false;

        data::Reference<Device> device_;

        std::uint32_t size_ = 0;

        std::size_t mapSize_ = 0;
        std::size_t mapOffset_ = 0;

        static VkBufferUsageFlags reverseMapUsage(std::uint32_t usage);

        friend class BufferMemoryMapping;
    };
}