#pragma once

#include <renderer/configuration.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

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
        PolygonTopology topology;

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
    struct Scissor {
        glm::ivec2 offset;
        glm::uvec2 extent;
    };

    // @brief The renderable area of the screen that a pipeline will target
    struct Viewport {
        // @brief The position of the top-left corner
        glm::fvec2 position = {};

        // @brief The extent (size) of the renderable region
        glm::fvec2 extent = {};

        // @brief The minimum depth of the region
        float minDepth = 0.0f;

        // @brief The maximum depth of the region
        float maxDepth = 1.0f;
    };

    struct PerFaceRasterisationState {
        // @brief Comparison used when depth testing
        CompareOperation depthComparison;

        // @brief Comparison used when stencil testing
        CompareOperation stencilComparison;

        // @brief Action to take when the stencil test fails
        ValueOperation stencilFailOperation;

        // @brief Action to take when the depth test fails
        // @note Depth testing always occurs after stencil testing
        ValueOperation depthFailOperation;

        // @brief Action to take when both depth and stencil pass
        ValueOperation passOperation;

        // @brief Comparison mask for stencil testing
        std::uint32_t stencilCompareMask = 0xF;

        // @brief Write mask for stencil testing
        std::uint32_t stencilWriteMask = 0xF;
    };

    struct RasterisationState {
        // @brief What winding direction the front face will have
        PolygonFaceWinding frontFaceWinding;

        // @brief What cull mode to use for polygons
        PolygonCullMode cullMode;

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
        BlendFactor sourceColourBlendFactor;

        // @brief Destination blend factor for the alpha channel
        BlendFactor destinationColourBlendFactor;

        // @brief Operation used when blending the alpha channel
        BlendOperation colourBlendOperation;

        // @brief Source blend factor for the alpha channel
        BlendFactor sourceAlphaBlendFactor;

        // @brief Destination blend factor for the alpha channel
        BlendFactor destinationAlphaBlendFactor;

        // @brief Operation used when blending the alpha channel
        BlendOperation alphaBlendOperation;
    };

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
        static DescriptorSetLayout create(const DescriptorSetLayoutCreateInfo& createInfo);
        static void destroy(DescriptorSetLayout& descriptorSetLayout);

    private:
        VkDescriptorSetLayout descriptorSetLayout_ = nullptr;
        Device* device_ = nullptr;

        friend class DescriptorSet;
        friend class DescriptorPool;
        friend class PipelineLayout;
    };

    struct PushConstantInputInfo {
        std::uint32_t sizeBytes;

        Flags stageFlags;
    };

    struct PipelineLayoutCreateInfo {
        Device& device;

        std::vector<DescriptorSetLayout> inputLayouts;
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
        std::vector<DescriptorSetLayout> layouts;
    };

    class DescriptorSet {
    private:
        VkDescriptorSet descriptorSet_ = nullptr;
        const DescriptorSetLayout* layout_ = nullptr;
        DescriptorPool* pool_ = nullptr;

        friend class DescriptorPool;
        friend class CommandBuffer;
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
        static DescriptorPool create(const DescriptorPoolCreateInfo& createInfo);
        static void destroy(DescriptorPool& descriptorPool);

        static std::vector<DescriptorSet> allocateDescriptorSets(DescriptorPool& descriptorPool, const DescriptorSetCreateInfo& createInfo);

        static void updateDescriptorSets(DescriptorPool& descriptorPool, std::vector<DescriptorSetUpdateInfo> updateInfos);

    private:
        VkDescriptorPool descriptorPool_ = nullptr;
        Device* device_ = nullptr;
    };

    class PipelineLayout {
    public:
        static PipelineLayout create(const PipelineLayoutCreateInfo& createInfo);
        static void destroy(PipelineLayout& pipelineLayout);

    private:
        VkPipelineLayout pipelineLayout_ = nullptr;
        Device* device_ = nullptr;

        friend class Device;
        friend class CommandBuffer;
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

    class Pipeline {
    public:
        static void destroy(Pipeline& pipeline);

    private:
        VkPipeline pipeline_ = nullptr;
        Device* device_ = nullptr;

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
        friend class CommandBuffer;
    };
}