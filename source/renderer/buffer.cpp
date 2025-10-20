#include <renderer/buffer.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>

namespace renderer {
    Buffer Buffer::create(const BufferCreateInfo& createInfo) {
        Buffer buffer;

        VmaMemoryUsage memoryUsage;
        VkMemoryPropertyFlags memoryProperties;

        switch (createInfo.memoryType) {
            case MemoryType::DEVICE_LOCAL:
                memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
                memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;

            case MemoryType::HOST_VISIBLE:
                memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
        }

        VmaAllocationCreateInfo allocationCreateInfo = {
            .flags = 0,
            .usage = memoryUsage,
            .requiredFlags = 0,
            .preferredFlags = memoryProperties,
            .memoryTypeBits = 0,
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.0,
        };

        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = createInfo.sizeBytes,
            .usage = BufferUsageFlags::mapFrom(createInfo.usageFlags),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
        };

        VmaAllocationInfo allocationInfo = {};

        if (vmaCreateBuffer(createInfo.device.allocator_, &bufferCreateInfo, &allocationCreateInfo, &buffer.buffer_, &buffer.allocation_, &allocationInfo) != VK_SUCCESS) {
            buffer.allocation_ = nullptr;
            buffer.buffer_ = nullptr;
        }
        else {
            buffer.device_ = &createInfo.device;

            auto& instance = buffer.device_->instance_;
            auto& properties = instance->memoryProperties_;

            VkMemoryPropertyFlags flags = properties.memoryTypes[allocationInfo.memoryType].propertyFlags;

            buffer.isHostCoherent_ = (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
            buffer.isHostVisible_ = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
            buffer.device_ = &createInfo.device;
            buffer.size_ = allocationInfo.size;
        }

        return buffer;
    }

    void Buffer::destroy(Buffer& buffer) {
        if (buffer.buffer_) {
            vmaDestroyBuffer(buffer.device_->allocator_, buffer.buffer_, buffer.allocation_);

            buffer.buffer_ = nullptr;
        }
    }

    BufferMapping Buffer::map(Buffer& buffer, std::uint64_t size, std::uint64_t offset) {
        BufferMapping mapping;

        mapping.offset = offset;

        if (!buffer.isHostCoherent_) {
            auto& instance = buffer.device_->instance_;
            auto& properties = instance->properties_;

            VkDeviceSize atomSize = properties.limits.nonCoherentAtomSize;

            mapping.alignedOffset = offset & ~(atomSize - 1);
            mapping.alignedSize = ((mapping.alignedOffset + size + atomSize - 1) & ~(atomSize - 1)) - mapping.alignedOffset;

            vmaInvalidateAllocation(buffer.device_->allocator_, buffer.allocation_, mapping.alignedOffset, mapping.alignedSize);
        }
        else {
            mapping.alignedOffset = offset;
            mapping.alignedSize = size;
        }

        void* data = nullptr;

        vmaMapMemory(buffer.device_->allocator_, buffer.allocation_, &data);

        mapping.data = {reinterpret_cast<std::uint8_t*>(data) + offset, size};

        return mapping;
    }

    void Buffer::unmap(Buffer& buffer, BufferMapping& mapping) {
        if (!buffer.isHostCoherent_) {
            vmaFlushAllocation(buffer.device_->allocator_, buffer.allocation_, mapping.alignedOffset, mapping.alignedSize);
        }

        vmaUnmapMemory(buffer.device_->allocator_, buffer.allocation_);
    }

    std::uint64_t Buffer::size(Buffer& buffer) {
        return buffer.size_;
    }

    bool Buffer::mappable(Buffer& buffer) {
        return buffer.isHostVisible_;
    }
}