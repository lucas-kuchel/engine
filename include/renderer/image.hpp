#pragma once

#include <renderer/configuration.hpp>

#include <cstdint>
#include <span>

#include <glm/glm.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Queue;
    class Device;
    class Instance;

    // @brief Creation information for the image
    struct ImageCreateInfo {
        Device& device;
        ImageType type;
        ImageFormat format;
        MemoryType memoryType;
        Flags usageFlags;

        glm::uvec3 extent;

        std::uint32_t sampleCount;
        std::uint32_t mipLevels;
        std::uint32_t arrayLayers;
    };

    struct ImageMapping {
        std::span<std::uint8_t> data;

        std::size_t offset = 0;

        std::size_t alignedSize = 0;
        std::size_t alignedOffset = 0;
    };

    class Image {
    public:
        static Image create(const ImageCreateInfo& createInfo);
        static void destroy(Image& image);

        static ImageMapping map(Image& image, std::size_t sizeBytes, std::size_t offsetBytes);
        static void unmap(Image& image, ImageMapping& mapping);

        static bool isMappable(Image& image);
        static ImageFormat getFormat(Image& image);
        static ImageType getType(Image& image);
        static std::size_t getSize(Image& image);
        static glm::uvec3 getExtent(Image& image);
        static std::uint32_t getSampleCount(Image& image);
        static std::uint32_t getMipLevels(Image& image);
        static std::uint32_t getArrayLayers(Image& image);

    private:
        VkImage image_ = nullptr;
        VmaAllocation allocation_ = nullptr;
        Device* device_ = nullptr;

        glm::uvec3 extent_;

        std::uint32_t sampleCount_;
        std::uint32_t mipLevels_;
        std::uint32_t arrayLayers_;

        VkImageType type_ = VK_IMAGE_TYPE_MAX_ENUM;
        VkFormat format_ = VK_FORMAT_MAX_ENUM;

        bool isHostVisible_ = false;
        bool isHostCoherent_ = false;

        std::size_t size_ = 0;

        static VkFormat mapFormat(ImageFormat format, renderer::Instance& instance);
        static ImageFormat reverseMapFormat(VkFormat format);

        static VkImageType mapType(ImageType type);
        static ImageType reverseMapType(VkImageType type);

        friend class Swapchain;
        friend class RenderPass;
        friend class Framebuffer;
        friend class ImageView;
        friend class CommandBuffer;
    };

    struct BufferImageCopyRegion {
        std::size_t bufferOffset;
        std::uint32_t bufferRowLength;
        std::uint32_t bufferImageHeight;
        std::uint32_t mipLevel;
        std::uint32_t baseArrayLayer;
        std::uint32_t arrayLayerCount;

        Flags imageAspectMask;

        glm::ivec3 imageOffset;
        glm::uvec3 imageExtent;
    };
}