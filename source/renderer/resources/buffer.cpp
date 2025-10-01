#include <renderer/resources/buffer.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>

namespace renderer {
    BufferMemoryMapping::~BufferMemoryMapping() {
        unmap();
    }

    std::span<std::uint32_t> BufferMemoryMapping::get() {
        if (!buffer_->isMapped_) {
            throw std::runtime_error("Call failed: renderer::BufferMemoryMapping::get(): Memory is no longer mapped");
        }

        return std::span<std::uint32_t>(reinterpret_cast<std::uint32_t*>(data_) + buffer_->mapOffset_, buffer_->mapSize_);
    }

    void BufferMemoryMapping::unmap() {
        auto& device = buffer_->device_.get();

        if (!buffer_->hostCoherent_) {
            vmaFlushAllocation(device.getVmaAllocator(), buffer_->memory_, buffer_->mapOffset_, buffer_->mapSize_);
        }

        if (buffer_->isMapped_) {
            vmaUnmapMemory(device.getVmaAllocator(), buffer_->memory_);

            buffer_->isMapped_ = false;
        }
    }

    bool BufferMemoryMapping::isMapped() const {
        return buffer_->isMapped_;
    }

    BufferMemoryMapping::BufferMemoryMapping(Buffer& buffer)
        : buffer_(buffer) {

        auto& device = buffer_->device_.get();

        if (!buffer_->hostCoherent_) {
            vmaInvalidateAllocation(device.getVmaAllocator(), buffer_->memory_, buffer_->mapOffset_, buffer_->mapSize_);
        }

        vmaMapMemory(device.getVmaAllocator(), buffer_->memory_, &data_);
    }

    Buffer::Buffer(const BufferCreateInfo& createInfo)
        : device_(createInfo.device), size_(createInfo.sizeBytes) {
        VmaMemoryUsage memoryUsage;
        VkMemoryPropertyFlags memoryProperties;

        switch (createInfo.type) {
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
            .usage = reverseMapUsage(createInfo.usage),
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

    BufferMemoryMapping Buffer::map(std::uint32_t size, std::uint32_t offset) {
        if (!hostVisible_) {
            throw std::runtime_error("Call failed: renderer::Buffer::map(): Cannot map memory that is not host visible");
        }

        isMapped_ = true;

        auto& properties = device_->getInstance().getVkPhysicalDeviceProperties();

        VkDeviceSize atomSize = properties.limits.nonCoherentAtomSize;

        mapOffset_ = offset & ~(atomSize - 1);
        mapSize_ = ((mapOffset_ + size + atomSize - 1) & ~(atomSize - 1)) - mapOffset_;

        return BufferMemoryMapping(*this);
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

    bool Buffer::isMapped() const {
        return isMapped_;
    }

    bool Buffer::isMappable() const {
        return hostVisible_ || hostCoherent_;
    }

    std::uint32_t Buffer::getSize() const {
        return size_;
    }

    VkBufferUsageFlags Buffer::reverseMapUsage(std::uint32_t usage) {
        VkBufferUsageFlags flags = 0;

        if (usage & BufferUsageFlags::VERTEX)
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (usage & BufferUsageFlags::INDEX)
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (usage & BufferUsageFlags::UNIFORM)
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (usage & BufferUsageFlags::STORAGE)
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (usage & BufferUsageFlags::TRANSFER_SOURCE)
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if (usage & BufferUsageFlags::TRANSFER_DESTINATION)
            flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        return flags;
    }
}