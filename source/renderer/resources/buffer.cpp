#include <renderer/resources/buffer.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>

namespace renderer {

    void Buffer::unmap() {
        if (isMapped_) {
            if (!hostCoherent_) {
                vmaFlushAllocation(device_->getVmaAllocator(), memory_, mapOffset_, mapSize_);
            }

            vmaUnmapMemory(device_->getVmaAllocator(), memory_);

            isMapped_ = false;
        }
    }

    bool Buffer::mapped() const {
        return isMapped_;
    }

    std::span<std::uint8_t> Buffer::map(std::uint64_t size, std::uint64_t offset) {
        if (!hostVisible_) {
            throw std::runtime_error("Call failed: renderer::Buffer::map(): Cannot map memory that is not host visible");
        }

        isMapped_ = true;

        if (!hostCoherent_) {
            auto& properties = device_->getInstance().getVkPhysicalDeviceProperties();

            VkDeviceSize atomSize = properties.limits.nonCoherentAtomSize;

            mapOffset_ = offset & ~(atomSize - 1);
            mapSize_ = ((mapOffset_ + size + atomSize - 1) & ~(atomSize - 1)) - mapOffset_;

            vmaInvalidateAllocation(device_->getVmaAllocator(), memory_, mapOffset_, mapSize_);
        }
        else {
            mapOffset_ = offset;
            mapSize_ = size;
        }

        void* data = nullptr;

        vmaMapMemory(device_->getVmaAllocator(), memory_, &data);

        return {reinterpret_cast<std::uint8_t*>(data) + offset, size};
    }

    Buffer::Buffer(const BufferCreateInfo& createInfo)
        : device_(createInfo.device), size_(createInfo.sizeBytes) {
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
            .size = size_,
            .usage = BufferUsageFlags::mapFrom(createInfo.usageFlags),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
        };

        VmaAllocationInfo allocationInfo = {};

        if (vmaCreateBuffer(device_->getVmaAllocator(), &bufferCreateInfo, &allocationCreateInfo, &buffer_, &memory_, &allocationInfo) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Buffer: Failed to allocate buffer memory");
        }

        auto& deviceMemoryProperties = device_->getInstance().getVkPhysicalDeviceMemoryProperties();

        VkMemoryPropertyFlags properties = deviceMemoryProperties.memoryTypes[allocationInfo.memoryType].propertyFlags;

        hostVisible_ = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        hostCoherent_ = (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
    }

    Buffer::~Buffer() {
        auto& device = device_.get();

        if (isMapped_) {

            if (hostCoherent_) {
                vmaFlushAllocation(device.getVmaAllocator(), memory_, mapOffset_, mapSize_);
            }

            vmaUnmapMemory(device.getVmaAllocator(), memory_);

            isMapped_ = false;
        }

        if (buffer_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(device.getVmaAllocator(), buffer_, memory_);
        }
    }

    VkBuffer& Buffer::getVkBuffer() {
        return buffer_;
    }

    const VkBuffer& Buffer::getVkBuffer() const {
        return buffer_;
    }

    VmaAllocation& Buffer::getVmaAllocation() {
        return memory_;
    }

    const VmaAllocation& Buffer::getVmaAllocation() const {
        return memory_;
    }

    bool Buffer::mappable() const {
        return hostVisible_ || hostCoherent_;
    }

    std::uint64_t Buffer::size() const {
        return size_;
    }
}