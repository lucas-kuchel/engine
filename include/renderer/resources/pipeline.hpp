#pragma once

#include <renderer/resources/config.hpp>

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
    class Buffer;
    class ImageView;
    class Sampler;

    // @brief Describes a shader stage of a pipeline
    struct ShaderStageInfo {
        ShaderModule& module;
        ShaderStage stage;

        // @brief Entry point for this shader stage
        // @note This is most commonly "main", particularly with SPIR-V compiled from GLSL
        std::string entry;
    };

    struct VertexInputBindingDescription {
        // @ brief If the buffer data is read per-vertex or per-instance
        VertexInputRate inputRate;

        // @brief What index to bind the buffer to
        std::uint32_t binding;

        // @brief How long each vertex data segment is
        std::uint32_t strideBytes;
    };

    struct VertexAttributeDescription {
        // @brief The format to read the vertex attribute in
        VertexAttributeFormat format;

        // @brief The buffer index to read the data from
        // @note Maps to a VertexInputBindingDescription's binding
        std::uint32_t binding;

        // @brief Location to bind the attribute to in the shader
        std::uint32_t location;
    };

    // @brief Describes the input assembly stage of the fixed-function pipeline
    struct PipelineInputAssemblyState {
        // @brief What topology to assemble vertices into
        PolygonTopology topology = PolygonTopology::TRIANGLE;

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

    struct DescriptorSetInputInfo {
        DescriptorInputType type;
        Flags stageFlags;

        std::uint32_t count;

        // @brief The binding index that the data comes from
        std::uint32_t binding;
    };

    struct DescriptorSetLayoutCreateInfo {
        Device& device;

        std::vector<DescriptorSetInputInfo> inputs;
    };

    class DescriptorSetLayout {
    public:
        DescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout(DescriptorSetLayout&&) noexcept = default;

        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) noexcept = default;

        [[nodiscard]] VkDescriptorSetLayout& getVkDescriptorSetLayout();

        [[nodiscard]] const VkDescriptorSetLayout& getVkDescriptorSetLayout() const;

    private:
        VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;

        data::Ref<Device> device_;
    };

    struct PushConstantInputInfo {
        std::uint32_t sizeBytes;

        Flags stageFlags;
    };

    struct PipelineLayoutCreateInfo {
        Device& device;

        std::vector<data::Ref<DescriptorSetLayout>> inputLayouts;
        std::vector<PushConstantInputInfo> pushConstants;
    };

    struct DescriptorSetBufferBinding {
        Buffer& buffer;

        std::uint64_t offsetBytes;
        std::uint64_t rangeBytes;
    };

    struct DescriptorSetImageBinding {
        ImageView& image;
        Sampler& sampler;
        ImageLayout layout;
    };

    struct DescriptorPoolSize {
        DescriptorInputType type;
        std::uint32_t count;
    };

    struct DescriptorPoolCreateInfo {
        Device& device;

        std::vector<DescriptorPoolSize> poolSizes;
        std::uint32_t maximumSetCount;
    };

    class DescriptorPool;

    struct DescriptorSetCreateInfo {
        std::vector<data::Ref<DescriptorSetLayout>> layouts;
    };

    class DescriptorSet {
    public:
        ~DescriptorSet();

        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet(DescriptorSet&&) noexcept = default;

        DescriptorSet& operator=(const DescriptorSet&) = delete;
        DescriptorSet& operator=(DescriptorSet&&) noexcept = default;

        [[nodiscard]] VkDescriptorSet& getVkDescriptorSet();

        [[nodiscard]] const VkDescriptorSet& getVkDescriptorSet() const;

    private:
        DescriptorSet(Device& device, const DescriptorSetLayout& layout, DescriptorPool& pool);

        VkDescriptorSet set_ = VK_NULL_HANDLE;

        data::Ref<Device> device_;
        data::Ref<const DescriptorSetLayout> layout_;
        data::Ref<DescriptorPool> pool_;

        friend class DescriptorPool;
    };

    struct DescriptorSetUpdateInfo {
        DescriptorSet& set;
        DescriptorInputType inputType;

        std::uint32_t binding;
        std::uint32_t arrayElement;

        std::vector<DescriptorSetBufferBinding> buffers;
        std::vector<DescriptorSetImageBinding> images;
    };

    class DescriptorPool {
    public:
        DescriptorPool(const DescriptorPoolCreateInfo& createInfo);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool(DescriptorPool&&) noexcept = default;

        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool& operator=(DescriptorPool&&) noexcept = default;

        [[nodiscard]] std::vector<DescriptorSet> allocateDescriptorSets(const DescriptorSetCreateInfo& createInfo);

        void updateDescriptorSets(std::vector<DescriptorSetUpdateInfo> updateInfos);

        [[nodiscard]] VkDescriptorPool& getVkDescriptorPool();

        [[nodiscard]] const VkDescriptorPool& getVkDescriptorPool() const;

    private:
        VkDescriptorPool pool_ = VK_NULL_HANDLE;

        data::Ref<Device> device_;
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

        data::Ref<Device> device_;
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

        VkPipeline pipeline_ = VK_NULL_HANDLE;

        data::Ref<Device> device_;

        static VkShaderStageFlagBits reverseMapShaderStage(ShaderStage stage);
        static VkVertexInputRate reverseMapVertexInputRate(VertexInputRate rate);

        static VkPrimitiveTopology reverseMapPrimitive(PolygonTopology topology);
        static VkCullModeFlags reverseMapCullMode(PolygonCullMode cullMode);
        static VkFrontFace reverseMapFrontFace(PolygonFaceWinding winding);

        static VkFormat reverseMapVertexAttributeFormat(VertexAttributeFormat format);
        static VkSampleCountFlagBits reverseMapSampleCount(std::uint32_t sampleCount);
        static VkCompareOp reverseMapCompareOperation(CompareOperation compare);
        static VkStencilOpState reverseMapStencilOperationState(PerFaceRasterisationState perFaceState);
        static VkBlendFactor reverseMapBlendFactor(BlendFactor factor);
        static VkBlendOp reverseMapBlendOperation(BlendOperation operation);
        static VkStencilOp reverseMapStencilOperation(ValueOperation operation);

        friend class Device;
    };
}