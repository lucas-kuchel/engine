#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

namespace renderer {
    // @brief Flags for configuring resources
    using Flags = std::uint32_t;

    // @brief Dictate how buffers will be used and optimised for
    struct BufferUsageFlags {
        enum {
            NONE = 0,

            // @brief Will be used as a vertex buffer
            VERTEX = 1 << 0,

            // @brief Will be used as an index buffer
            INDEX = 1 << 1,

            // @brief Will be used as a uniform buffer
            UNIFORM = 1 << 2,

            // @brief Will be used as a storage buffer
            STORAGE = 1 << 3,

            // @brief Will support transfer operations from this buffer
            TRANSFER_SOURCE = 1 << 4,

            // @brief Will support transfer operations into this buffer
            TRANSFER_DESTINATION = 1 << 5,
        };

        static VkFlags mapFrom(Flags flags);
    };

    struct ImageAspectFlags {
        enum {
            COLOUR = 1 << 0,
            DEPTH = 1 << 1,
            STENCIL = 1 << 2,
        };

        static VkFlags mapFrom(Flags flags);
    };

    // @brief Flags for fence creation
    struct FenceCreateFlags {
        enum {
            NONE = 0,

            // @brief Start the fence as signalled
            START_SIGNALLED = 1 << 0,
        };

        static VkFlags mapFrom(Flags flags);
    };

    // @brief Image usage flags
    struct ImageUsageFlags {
        enum {
            NONE = 0,
            TRANSFER_SOURCE = 1 << 0,
            TRANSFER_DESTINATION = 1 << 1,
            SAMPLED = 1 << 2,
            STORAGE = 1 << 3,
            COLOR_ATTACHMENT = 1 << 4,
            DEPTH_STENCIL_ATTACHMENT = 1 << 5,
        };

        static VkFlags mapFrom(Flags flags);
    };

    enum class ImageLayout {
        UNDEFINED = 1 << 0,
        PREINITIALIZED = 1 << 1,
        COLOR_ATTACHMENT_OPTIMAL = 1 << 2,
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 1 << 3,
        SHADER_READ_ONLY_OPTIMAL = 1 << 4,
        TRANSFER_SOURCE_OPTIMAL = 1 << 5,
        TRANSFER_DESTINATION_OPTIMAL = 1 << 6,
        GENERAL = 1 << 7,
        PRESENT_SOURCE = 1 << 8,
    };

    struct DescriptorShaderStageFlags {
        enum {
            NONE = 0,
            VERTEX = 1 << 0,
            FRAGMENT = 1 << 1,
        };

        static VkFlags mapFrom(Flags flags);
    };

    struct StencilFaceFlags {
        enum {
            NONE = 0,
            FRONT = 1 << 0,
            BACK = 1 << 1,
            BOTH = FRONT | BACK,
        };

        static VkFlags mapFrom(Flags flags);
    };

    // @brief Queue capabilites required
    struct QueueFlags {
        enum {
            NONE = 0,

            // @brief Queue must support presentation to a display
            PRESENT = 1 << 0,

            // @brief Queue must support transferring data on the GPU
            TRANSFER = 1 << 1,

            // @brief Queue must support GPGPU compute operations
            COMPUTE = 1 << 2,

            // @brief Queue must support rendering and general-purpose graphics operations
            GRAPHICS = 1 << 3,
        };

        static VkFlags mapFrom(Flags flags);
    };

    struct PipelineStageFlags {
        enum {
            NONE = 0,
            TOP_OF_PIPE = 1 << 0,
            DRAW_INDIRECT = 1 << 1,
            VERTEX_INPUT = 1 << 2,
            VERTEX_SHADER = 1 << 3,
            FRAGMENT_SHADER = 1 << 4,
            EARLY_FRAGMENT_TESTS = 1 << 5,
            LATE_FRAGMENT_TESTS = 1 << 6,
            COLOR_ATTACHMENT_OUTPUT = 1 << 7,
            TRANSFER = 1 << 8,
            BOTTOM_OF_PIPE = 1 << 9,
            HOST = 1 << 10,
            ALL_GRAPHICS = 1 << 11,
            ALL_COMMANDS = 1 << 12,
        };

        static VkFlags mapFrom(Flags flags);
    };

    struct AccessFlags {
        enum {
            NONE = 0,
            INDIRECT_COMMAND_READ = 1 << 0,
            INDEX_READ = 1 << 1,
            VERTEX_ATTRIBUTE_READ = 1 << 2,
            UNIFORM_READ = 1 << 3,
            INPUT_ATTACHMENT_READ = 1 << 4,
            SHADER_READ = 1 << 5,
            SHADER_WRITE = 1 << 6,
            COLOR_ATTACHMENT_READ = 1 << 7,
            COLOR_ATTACHMENT_WRITE = 1 << 8,
            DEPTH_STENCIL_ATTACHMENT_READ = 1 << 9,
            DEPTH_STENCIL_ATTACHMENT_WRITE = 1 << 10,
            TRANSFER_READ = 1 << 11,
            TRANSFER_WRITE = 1 << 12,
            HOST_READ = 1 << 13,
            HOST_WRITE = 1 << 14,
            MEMORY_READ = 1 << 15,
            MEMORY_WRITE = 1 << 16,
        };

        static VkFlags mapFrom(Flags flags);
    };

    // @brief Describes how allocated memory can be accessed
    enum class MemoryType {
        // @brief Visible to the CPU
        // @note Most effective for smaller and CPU-accessible data
        HOST_VISIBLE,

        // @brief Not visible to the CPU
        // @note Most efficient for large CPU-inaccessible data
        DEVICE_LOCAL,
    };

    enum class Filter {
        LINEAR,
        NEAREST,
    };

    enum class AddressMode {
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE,
    };

    enum class BorderColour {
        FLOAT_TRANSPARENT_BLACK,
        FLOAT_OPAQUE_BLACK,
        FLOAT_OPAQUE_WHITE,
        INT_TRANSPARENT_BLACK,
        INT_OPAQUE_BLACK,
        INT_OPAQUE_WHITE,
    };

    enum class MipmapMode {
        NEAREST,
        LINEAR,
    };

    // @brief Image dimensionality
    enum class ImageType {
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
    };

    // @brief Image view dimensionality
    enum class ImageViewType {
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
    };

    // @brief Image data format
    enum class ImageFormat {
        R8_UNORM,
        R8G8_UNORM,
        R8G8B8_UNORM,
        R8G8B8A8_UNORM,
        B8G8R8A8_UNORM,

        B8G8R8A8_SRGB,

        R16G16B16A16_SFLOAT,
        R32G32B32A32_SFLOAT,

        DEPTH_STENCIL,
        DEPTH_ONLY,
        STENCIL_ONLY,
    };

    // @brief What to do when loading a frame attachment
    enum class LoadOperation {
        LOAD,
        CLEAR,
        DONT_CARE,
    };

    // @brief What to do when storing a frame attachment
    enum class StoreOperation {
        STORE,
        DONT_CARE,
    };

    // @brief What index type to use for indexed draw calls
    enum class IndexType {
        UINT16,
        UINT32,
    };

    // @brief What type of input is expected for a descriptor
    enum class DescriptorInputType {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        IMAGE_SAMPLER,
    };

    // @brief Possible blend factors used for combining source and destination values
    enum class BlendFactor {
        ZERO,
        ONE,
        SOURCE_COLOUR,
        ONE_MINUS_SOURCE_COLOUR,
        DESTINATION_COLOUR,
        ONE_MINUS_DESTINATION_COLOUR,
        SOURCE_ALPHA,
        ONE_MINUS_SOURCE_ALPHA,
        DESTINATION_ALPHA,
        ONE_MINUS_DESTINATION_ALPHA,
        CONSTANT_COLOUR,
        ONE_MINUS_CONSTANT_COLOUR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        SOURCE_ALPHA_SATURATE,
    };

    // @brief Operations used when blending colour or alpha channels
    enum class BlendOperation {
        ADD,
        SUBTRACT,
        REVERSE_SUBTRACT,
        MIN,
        MAX,
    };

    // @brief Defines the winding order for points of a polygon
    enum class PolygonFaceWinding {
        CLOCKWISE,
        ANTICLOCKWISE,
    };

    // @brief Defines the culling mode of a polygon
    enum class PolygonCullMode {
        NEVER,
        FRONT,
        BACK,
        ALWAYS,
    };

    // @brief What operation a given system should target
    enum class DeviceOperation {
        GRAPHICS,
        COMPUTE,
    };

    // @brief General-purpose comparison operations
    enum class CompareOperation {
        EQUAL,
        NOT_EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,
        ALWAYS,
        NEVER,
    };

    // @brief General-purpose value-setting operations
    enum class ValueOperation {
        KEEP,
        ZERO,
        REPLACE,
        INCREMENT_AND_CLAMP,
        DECREMENT_AND_CLAMP,
        INVERT,
        INCREMENT_AND_WRAP,
        DECREMENT_AND_WRAP,
    };

    // @brief The topology to render a polygon as
    enum class PolygonTopology {
        POINT,
        LINE,
        TRIANGLE,
        LINE_STRIP,
        TRIANGLE_STRIP,
    };

    // @brief The format to read a vertex attribute as
    enum class VertexAttributeFormat {
        R32_FLOAT,
        R32G32_FLOAT,
        R32G32B32_FLOAT,
        R32G32B32A32_FLOAT,

        R32_INT,
        R32G32_INT,
        R32G32B32_INT,
        R32G32B32A32_INT,

        R32_UINT,
        R32G32_UINT,
        R32G32B32_UINT,
        R32G32B32A32_UINT,
    };

    // @brief Determines if a vertex buffer's data is read per-vertex or per-instance
    enum class VertexInputRate {
        PER_VERTEX,
        PER_INSTANCE,
    };

    enum class ShaderStage {
        VERTEX,
        FRAGMENT,
    };
}