#pragma once

#include <data/colour.hpp>
#include <data/range.hpp>
#include <data/rect.hpp>
#include <data/references.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class ShaderModule;
    class Device;
    class RenderPass;

    // TODO: add push constants and descriptor sets for uniform buffers, storage buffers and samplers

    enum class ShaderStage {
        VERTEX = 0,
        FRAGMENT = 1 << 0,
    };

    // @brief Describes a shader stage of a pipeline
    struct ShaderStageInfo {
        ShaderModule& module;

        ShaderStage stage;

        // @brief Entry point for this shader stage
        // @note This is commonly "main"
        std::string entry;
    };

    // @brief Determines if a vertex buffer's data is read per-vertex or per-instance
    enum class VertexInputRate {
        PER_VERTEX,
        PER_INSTANCE,
    };

    struct VertexInputBindingDescription {
        // @brief What index to bind the buffer to
        std::uint32_t binding;

        // @brief How long each vertex data segment is
        std::uint32_t strideBytes;

        // @ brief If the buffer data is read per-vertex or per-instance
        VertexInputRate inputRate;
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

    struct VertexAttributeDescription {
        // @brief The format to read the vertex attribute in
        VertexAttributeFormat format;

        // @brief Location to bind the attribute to in the shader
        std::uint32_t location;

        // @brief The buffer index to read the data from
        // @note Maps to a VertexInputBindingDescription's binding
        std::uint32_t binding;
    };

    // @brief The primitive topology to render
    enum class RasterisationPrimitive {
        POINT,
        LINE,
        TRIANGLE,
        LINE_STRIP,
        TRIANGLE_STRIP,
    };

    // @brief Describes the input assembly stage of the fixed-function pipeline
    struct PipelineInputAssemblyState {
        // @brief What topology to assemble vertices into
        RasterisationPrimitive topology = RasterisationPrimitive::TRIANGLE;

        // @brief If strip-type primitives should be restarted when a certain value is seen
        // @note The restart value should be in the index buffer. 0xFFFF for 16-bit indices, OxFFFFFFFF for 32-bit indices
        bool primitiveRestart = false;
    };

    // @brief Describes all state related to vertex buffer inputs
    struct PipelineVertexInputState {
        std::vector<VertexInputBindingDescription> bindings;
        std::vector<VertexAttributeDescription> attributes;
    };

    // @brief Describes the area of a viewport to be used
    using Scissor = data::Rect2D<std::int32_t, std::uint32_t>;

    // @brief The renderable area of the screen that a pipeline will target
    struct Viewport {
        // @brief The position of the top-left corner
        data::Position2D<float> position = {};

        // @brief The extent (size) of the renderable region
        data::Extent2D<float> extent = {};

        // @brief The depth range of the region
        data::Range<float> depth = {};
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

    struct PerFaceRasterisationState {
        // @brief Comparison used when depth testing
        CompareOperation depthComparison = CompareOperation::NEVER;

        // @brief Comparison used when stencil testing
        CompareOperation stencilComparison = CompareOperation::NEVER;

        // @brief Action to take when the stencil test fails
        ValueOperation stencilFailOperation = ValueOperation::ZERO;

        // @brief Action to take when the depth test fails
        // @note Depth testing always occurs after stencil testing
        ValueOperation depthFailOperation = ValueOperation::ZERO;

        // @brief Action to take when both depth and stencil pass
        ValueOperation passOperation = ValueOperation::ZERO;

        // @brief Comparison mask for stencil testing
        std::uint32_t stencilCompareMask = 0xF;

        // @brief Write mask for stencil testing
        std::uint32_t stencilWriteMask = 0xF;
    };

    struct RasterisationState {
        // @brief What winding direction the front face will have
        PolygonFaceWinding frontFaceWinding = PolygonFaceWinding::CLOCKWISE;

        // @brief What cull mode to use for polygons
        PolygonCullMode cullMode = PolygonCullMode::NEVER;

        // @brief Frontface rasterisation state
        PerFaceRasterisationState frontface = {};

        // @brief Backface rasterisation state
        PerFaceRasterisationState backface = {};

        // @brief If depth values should be clamped
        bool depthClampEnable = false;

        // @brief If depth values should be read and tested against
        bool depthTestEnable = false;

        // @brief If depth values should be written to the depth texture
        bool depthWriteEnable = false;

        // @brief If depth bounds testing should occur
        bool depthBoundsTestEnable = false;

        // @brief If stencil testing should be performed
        bool stencilTestEnable = false;
    };

    // @brief Describes how rasterised fragments are sampled
    struct MultisampleState {
        // @brief Number of samples per pixel (1 = no MSAA)
        std::uint32_t sampleCount = 1;

        // @brief If enabled, run the fragment shader per-sample instead of per-pixel
        bool sampleShadingEnable = false;

        // @brief If enabled, use the fragment's alpha to influence sample coverage
        bool alphaToCoverageEnable = false;

        // @brief If enabled, force the alpha channel to 1.0 after coverage
        bool alphaToOneEnable = false;

        // @brief Minimum fraction of samples that must be shaded
        // @note Only relevant if sampleShadingEnable is true
        float minSampleShading = 0.0;
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

    // @brief Describes how a single render target should blend fragments
    struct ColourBlendAttachment {
        // @brief If blending is enabled for this attachment
        bool blendEnable = false;

        // @brief Source blend factor for the alpha channel
        BlendFactor sourceColourBlendFactor = BlendFactor::ONE;

        // @brief Destination blend factor for the alpha channel
        BlendFactor destinationColourBlendFactor = BlendFactor::ZERO;

        // @brief Operation used when blending the alpha channel
        BlendOperation colourBlendOperation = BlendOperation::ADD;

        // @brief Source blend factor for the alpha channel
        BlendFactor sourceAlphaBlendFactor = BlendFactor::ONE;

        // @brief Destination blend factor for the alpha channel
        BlendFactor destinationAlphaBlendFactor = BlendFactor::ZERO;

        // @brief Operation used when blending the alpha channel
        BlendOperation alphaBlendOperation = BlendOperation::ADD;
    };

    using BlendConstants = data::ColourRGBA;

    // @brief Describes the blend state of all colour attachments
    struct ColourBlendState {
        std::vector<ColourBlendAttachment> attachments;
    };

    enum class StencilFaces {
        FRONT,
        BACK,
        BOTH,
    };

    enum class IndexType {
        U16,
        U32,
    };

    enum class ShaderInputType {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        DYNAMIC_UNIFORM_BUFFER,
        DYNAMIC_STORAGE_BUFFER,
    };

    struct ShaderStageFlags {
        enum {
            VERTEX = 1 << 0,
            FRAGMENT = 1 << 1,
        };
    };

    struct ShaderInputInfo {
        ShaderInputType type;

        std::uint32_t count;

        // @brief The buffer binding index that the data comes from
        std::uint32_t binding;
        std::uint32_t stageFlags;
    };

    struct ShaderInputLayoutCreateInfo {
        Device& device;

        std::vector<ShaderInputInfo> inputs;
    };

    class ShaderInputLayout {
    public:
        ShaderInputLayout(const ShaderInputLayoutCreateInfo& createInfo);
        ~ShaderInputLayout();

        ShaderInputLayout(const ShaderInputLayout&) = delete;
        ShaderInputLayout(ShaderInputLayout&&) noexcept = default;

        ShaderInputLayout& operator=(const ShaderInputLayout&) = delete;
        ShaderInputLayout& operator=(ShaderInputLayout&&) noexcept = default;

        [[nodiscard]] VkDescriptorSetLayout& getVkDescriptorSetLayout();

        [[nodiscard]] const VkDescriptorSetLayout& getVkDescriptorSetLayout() const;

    private:
        VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;

        data::Reference<Device> device_;
    };

    struct PushConstantInfo {
        std::uint32_t sizeBytes;
        std::uint32_t stageFlags;
    };

    struct PipelineLayoutCreateInfo {
        Device& device;

        std::vector<data::Reference<ShaderInputLayout>> inputLayouts;
        std::vector<PushConstantInfo> pushConstants;
    };

    class PipelineLayout {
    public:
        PipelineLayout(const PipelineLayoutCreateInfo& createInfo);
        ~PipelineLayout();

        PipelineLayout(const PipelineLayout&) = delete;
        PipelineLayout(PipelineLayout&&) noexcept = default;

        PipelineLayout& operator=(const PipelineLayout&) = delete;
        PipelineLayout& operator=(PipelineLayout&&) noexcept = default;

        [[nodiscard]] VkPipelineLayout& getVkPipelineLayout();

        [[nodiscard]] const VkPipelineLayout& getVkPipelineLayout() const;

    private:
        VkPipelineLayout layout_ = VK_NULL_HANDLE;

        data::Reference<Device> device_;
    };

    // @brief Creation information for a pipeline
    struct PipelineCreateInfo {
        RenderPass& renderPass;
        PipelineLayout& layout;

        // @brief Subpass of the render pass to target
        std::uint32_t subpassIndex;

        // @brief Shader stages for this pipeline
        std::vector<ShaderStageInfo> shaderStages;

        // @brief Vertex input descriptions
        PipelineVertexInputState vertexInput;

        // @brief Input assembly state
        PipelineInputAssemblyState inputAssembly;

        std::uint32_t viewportCount;
        std::uint32_t scissorCount;

        // @brief Rasterisation, depth, and stencil state
        RasterisationState rasterisation;

        // @brief Multisampling state
        MultisampleState multisample;

        // @brief Colour blending state
        ColourBlendState colourBlend;
    };

    // @brief Represents a pipeline to be used when rendering
    class Pipeline {
    public:
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline(Pipeline&&) noexcept = default;

        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline& operator=(Pipeline&&) noexcept = default;

        [[nodiscard]] VkPipeline& getVkPipeline();

        [[nodiscard]] const VkPipeline& getVkPipeline() const;

    private:
        Pipeline(Device& device);

        data::Reference<Device> device_;

        VkPipeline pipeline_ = VK_NULL_HANDLE;

        static VkShaderStageFlagBits reverseMapShaderStage(ShaderStage stage);
        static VkVertexInputRate reverseMapVertexInputRate(VertexInputRate rate);
        static VkPrimitiveTopology reverseMapPrimitive(RasterisationPrimitive topology);
        static VkFormat reverseMapVertexAttributeFormat(VertexAttributeFormat format);
        static VkCullModeFlags reverseMapCullMode(PolygonCullMode cullMode);
        static VkFrontFace reverseMapFrontFace(PolygonFaceWinding winding);
        static VkSampleCountFlagBits reverseMapSampleCount(std::uint32_t sampleCount);
        static VkCompareOp reverseMapCompareOperation(CompareOperation compare);
        static VkStencilOpState reverseMapStencilOperationState(PerFaceRasterisationState perFaceState);
        static VkBlendFactor reverseMapBlendFactor(BlendFactor factor);
        static VkBlendOp reverseMapBlendOperation(BlendOperation operation);
        static VkStencilOp reverseMapStencilOperation(ValueOperation operation);

        friend class Device;
    };
}