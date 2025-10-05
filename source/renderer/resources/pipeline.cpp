#include <renderer/resources/buffer.hpp>
#include <renderer/resources/pipeline.hpp>
#include <renderer/resources/sampler.hpp>

#include <renderer/device.hpp>

namespace renderer {
    DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo)
        : device_(createInfo.device) {

        std::vector<VkDescriptorSetLayoutBinding> bindings(createInfo.inputs.size());

        for (std::size_t i = 0; i < bindings.size(); i++) {
            auto& binding = bindings[i];
            auto& input = createInfo.inputs[i];

            VkShaderStageFlags flags = 0;

            if (input.stageFlags & DescriptorShaderStageFlags::VERTEX) {
                flags |= VK_SHADER_STAGE_VERTEX_BIT;
            }

            if (input.stageFlags & DescriptorShaderStageFlags::FRAGMENT) {
                flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

            switch (input.type) {
                case DescriptorInputType::UNIFORM_BUFFER:
                    descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;

                case DescriptorInputType::STORAGE_BUFFER:
                    descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;

                case DescriptorInputType::IMAGE_SAMPLER:
                    descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
            }

            binding = {
                .binding = input.binding,
                .descriptorType = descriptorType,
                .descriptorCount = input.count,
                .stageFlags = flags,
                .pImmutableSamplers = nullptr,
            };
        }

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<std::uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };

        if (vkCreateDescriptorSetLayout(device_->getVkDevice(), &layoutCreateInfo, nullptr, &layout_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::DescriptorSetLayout: Failed to create shader input layout");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        if (layout_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_->getVkDevice(), layout_, nullptr);
        }
    }

    VkDescriptorSetLayout& DescriptorSetLayout::getVkDescriptorSetLayout() {
        return layout_;
    }

    const VkDescriptorSetLayout& DescriptorSetLayout::getVkDescriptorSetLayout() const {
        return layout_;
    }

    DescriptorSet::~DescriptorSet() {
    }

    VkDescriptorSet& DescriptorSet::getVkDescriptorSet() {
        return set_;
    }

    const VkDescriptorSet& DescriptorSet::getVkDescriptorSet() const {
        return set_;
    }

    DescriptorSet::DescriptorSet(Device& device, const DescriptorSetLayout& layout, DescriptorPool& pool)
        : device_(device), layout_(layout), pool_(pool) {
    }

    DescriptorPool::DescriptorPool(const DescriptorPoolCreateInfo& createInfo)
        : device_(createInfo.device) {
        std::vector<VkDescriptorPoolSize> poolSizes(createInfo.poolSizes.size());

        for (std::size_t i = 0; i < poolSizes.size(); i++) {
            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

            switch (createInfo.poolSizes[i].type) {
                case DescriptorInputType::UNIFORM_BUFFER:
                    descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;

                case DescriptorInputType::STORAGE_BUFFER:
                    descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;

                case DescriptorInputType::IMAGE_SAMPLER:
                    descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
            }

            poolSizes[i] = {
                .type = descriptorType,
                .descriptorCount = createInfo.poolSizes[i].count,
            };
        }

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = createInfo.maximumSetCount,
            .poolSizeCount = static_cast<std::uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        if (vkCreateDescriptorPool(device_->getVkDevice(), &poolCreateInfo, nullptr, &pool_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::DescriptorPool: Failed to create descriptor pool");
        }
    }

    DescriptorPool::~DescriptorPool() {
        if (pool_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device_->getVkDevice(), pool_, nullptr);
        }
    }

    std::vector<DescriptorSet> DescriptorPool::allocateDescriptorSets(const DescriptorSetCreateInfo& createInfo) {
        std::vector<VkDescriptorSet> descriptorSets(createInfo.layouts.size());
        std::vector<VkDescriptorSetLayout> layouts(createInfo.layouts.size());

        for (std::size_t i = 0; i < layouts.size(); i++) {
            layouts[i] = createInfo.layouts[i]->getVkDescriptorSetLayout();
        }

        VkDescriptorSetAllocateInfo allocationInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = pool_,
            .descriptorSetCount = static_cast<std::uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data(),
        };

        if (vkAllocateDescriptorSets(device_->getVkDevice(), &allocationInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::DescriptorPool::allocateDescriptorSets(): Failed to allocate descriptor sets");
        }

        std::vector<DescriptorSet> sets;

        sets.reserve(createInfo.layouts.size());

        for (std::size_t i = 0; i < descriptorSets.size(); i++) {
            auto& descriptorSet = descriptorSets[i];
            auto& descriptorSetLayout = createInfo.layouts[i];

            sets.push_back(DescriptorSet(device_.get(), descriptorSetLayout.get(), *this));

            auto& set = sets.back();

            set.set_ = descriptorSet;
        }

        return sets;
    }

    void DescriptorPool::updateDescriptorSets(std::vector<DescriptorSetUpdateInfo> updateInfos) {
        std::vector<VkWriteDescriptorSet> writes(updateInfos.size());

        std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos(updateInfos.size());
        std::vector<std::vector<VkDescriptorImageInfo>> imageInfos(updateInfos.size());

        for (std::size_t i = 0; i < writes.size(); i++) {
            bufferInfos[i].resize(updateInfos[i].buffers.size());
            imageInfos[i].resize(updateInfos[i].images.size());

            for (std::size_t j = 0; j < bufferInfos[i].size(); j++) {
                bufferInfos[i][j] = {
                    .buffer = updateInfos[i].buffers[j].buffer.getVkBuffer(),
                    .offset = updateInfos[i].buffers[j].offsetBytes,
                    .range = updateInfos[i].buffers[j].rangeBytes,
                };
            }

            for (std::size_t j = 0; j < imageInfos[i].size(); j++) {
                struct FlagMap {
                    ImageLayout layout;
                    VkImageLayout vkLayout;
                };

                constexpr FlagMap flagMapping[] = {
                    {ImageLayout::UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED},
                    {ImageLayout::PREINITIALIZED, VK_IMAGE_LAYOUT_PREINITIALIZED},
                    {ImageLayout::COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                    {ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
                    {ImageLayout::SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                    {ImageLayout::TRANSFER_SOURCE_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
                    {ImageLayout::TRANSFER_DESTINATION_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
                    {ImageLayout::GENERAL, VK_IMAGE_LAYOUT_GENERAL},
                    {ImageLayout::PRESENT_SOURCE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
                };

                VkImageLayout layout;

                for (auto& mapping : flagMapping) {
                    if (updateInfos[i].images[j].layout == mapping.layout) {
                        layout = mapping.vkLayout;
                        break;
                    }
                }

                imageInfos[i][j] = {
                    .sampler = updateInfos[i].images[j].sampler.getVkSampler(),
                    .imageView = updateInfos[i].images[j].image.getVkImageView(),
                    .imageLayout = layout,
                };
            }

            VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

            switch (updateInfos[i].inputType) {
                case DescriptorInputType::UNIFORM_BUFFER:
                    descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;

                case DescriptorInputType::STORAGE_BUFFER:
                    descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;

                case DescriptorInputType::IMAGE_SAMPLER:
                    descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
            }

            writes[i] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = updateInfos[i].set.getVkDescriptorSet(),
                .dstBinding = updateInfos[i].binding,
                .dstArrayElement = updateInfos[i].arrayElement,
                .descriptorCount = static_cast<std::uint32_t>(bufferInfos[i].size() + imageInfos[i].size()),
                .descriptorType = descriptorType,
                .pImageInfo = imageInfos[i].data(),
                .pBufferInfo = bufferInfos[i].data(),
                .pTexelBufferView = nullptr,
            };
        }

        vkUpdateDescriptorSets(device_->getVkDevice(), static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    VkDescriptorPool& DescriptorPool::getVkDescriptorPool() {
        return pool_;
    }

    const VkDescriptorPool& DescriptorPool::getVkDescriptorPool() const {
        return pool_;
    }

    PipelineLayout::PipelineLayout(const PipelineLayoutCreateInfo& createInfo)
        : device_(createInfo.device) {
        std::vector<VkDescriptorSetLayout> descriptorSets(createInfo.inputLayouts.size());
        std::vector<VkPushConstantRange> pushConstants(createInfo.pushConstants.size());

        for (std::size_t i = 0; i < descriptorSets.size(); i++) {
            descriptorSets[i] = createInfo.inputLayouts[i]->getVkDescriptorSetLayout();
        }

        std::uint32_t pushConstantOffset = 0;

        for (std::size_t i = 0; i < pushConstants.size(); i++) {
            auto& info = pushConstants[i];
            auto& pushConstant = createInfo.pushConstants[i];

            VkShaderStageFlags flags = 0;

            if (pushConstant.stageFlags & DescriptorShaderStageFlags::VERTEX) {
                flags |= VK_SHADER_STAGE_VERTEX_BIT;
            }

            if (pushConstant.stageFlags & DescriptorShaderStageFlags::FRAGMENT) {
                flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            info = {
                .stageFlags = flags,
                .offset = pushConstantOffset,
                .size = pushConstant.sizeBytes,
            };

            pushConstantOffset += pushConstant.sizeBytes;
        }

        VkPipelineLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<std::uint32_t>(descriptorSets.size()),
            .pSetLayouts = descriptorSets.data(),
            .pushConstantRangeCount = static_cast<std::uint32_t>(pushConstants.size()),
            .pPushConstantRanges = pushConstants.data(),
        };

        if (vkCreatePipelineLayout(device_->getVkDevice(), &layoutCreateInfo, nullptr, &layout_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::PipelineLayout: Failed to create pipeline layout");
        }
    }

    PipelineLayout::~PipelineLayout() {
        if (layout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_->getVkDevice(), layout_, nullptr);
        }
    }

    VkPipelineLayout& PipelineLayout::getVkPipelineLayout() {
        return layout_;
    }

    const VkPipelineLayout& PipelineLayout::getVkPipelineLayout() const {
        return layout_;
    }

    Pipeline::~Pipeline() {
        if (pipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_->getVkDevice(), pipeline_, nullptr);
        }
    }

    VkPipeline& Pipeline::getVkPipeline() {
        return pipeline_;
    }

    const VkPipeline& Pipeline::getVkPipeline() const {
        return pipeline_;
    }

    Pipeline::Pipeline(Device& device)
        : device_(device) {
    }

    VkShaderStageFlagBits Pipeline::reverseMapShaderStage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;

            case ShaderStage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapShaderStage: Shader type is unsupported");
        }
    }

    VkVertexInputRate Pipeline::reverseMapVertexInputRate(VertexInputRate rate) {
        switch (rate) {
            case VertexInputRate::PER_VERTEX:
                return VK_VERTEX_INPUT_RATE_VERTEX;

            case VertexInputRate::PER_INSTANCE:
                return VK_VERTEX_INPUT_RATE_INSTANCE;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapVertexInputRate: Input rate is unknown");
        }
    }

    VkPrimitiveTopology Pipeline::reverseMapPrimitive(PolygonTopology topology) {
        switch (topology) {
            case PolygonTopology::POINT:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

            case PolygonTopology::LINE:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

            case PolygonTopology::TRIANGLE:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            case PolygonTopology::LINE_STRIP:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

            case PolygonTopology::TRIANGLE_STRIP:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapPrimitive: Primitive is unknown");
        }
    }

    VkFormat Pipeline::reverseMapVertexAttributeFormat(VertexAttributeFormat format) {
        switch (format) {
            case VertexAttributeFormat::R32_FLOAT:
                return VK_FORMAT_R32_SFLOAT;

            case VertexAttributeFormat::R32G32_FLOAT:
                return VK_FORMAT_R32G32_SFLOAT;

            case VertexAttributeFormat::R32G32B32_FLOAT:
                return VK_FORMAT_R32G32B32_SFLOAT;

            case VertexAttributeFormat::R32G32B32A32_FLOAT:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case VertexAttributeFormat::R32_INT:
                return VK_FORMAT_R32_SINT;

            case VertexAttributeFormat::R32G32_INT:
                return VK_FORMAT_R32G32_SINT;

            case VertexAttributeFormat::R32G32B32_INT:
                return VK_FORMAT_R32G32B32_SINT;

            case VertexAttributeFormat::R32G32B32A32_INT:
                return VK_FORMAT_R32G32B32A32_SINT;

            case VertexAttributeFormat::R32_UINT:
                return VK_FORMAT_R32_UINT;

            case VertexAttributeFormat::R32G32_UINT:
                return VK_FORMAT_R32G32_UINT;

            case VertexAttributeFormat::R32G32B32_UINT:
                return VK_FORMAT_R32G32B32_UINT;

            case VertexAttributeFormat::R32G32B32A32_UINT:
                return VK_FORMAT_R32G32B32A32_UINT;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapVertexAttributeFormat: Format is unknown");
        }
    }

    VkCullModeFlags Pipeline::reverseMapCullMode(PolygonCullMode cullMode) {
        switch (cullMode) {
            case PolygonCullMode::NEVER:
                return VK_CULL_MODE_NONE;

            case PolygonCullMode::FRONT:
                return VK_CULL_MODE_FRONT_BIT;

            case PolygonCullMode::BACK:
                return VK_CULL_MODE_BACK_BIT;

            case PolygonCullMode::ALWAYS:
                return VK_CULL_MODE_FRONT_AND_BACK;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapCullMode: Cull mode is unknown");
        }
    }

    VkFrontFace Pipeline::reverseMapFrontFace(PolygonFaceWinding winding) {
        switch (winding) {
            case PolygonFaceWinding::CLOCKWISE:
                return VK_FRONT_FACE_CLOCKWISE;

            case PolygonFaceWinding::ANTICLOCKWISE:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapFrontFace: Winding is unknown");
        }
    }

    VkSampleCountFlagBits Pipeline::reverseMapSampleCount(std::uint32_t sampleCount) {
        switch (sampleCount) {
            case 1:
                return VK_SAMPLE_COUNT_1_BIT;

            case 2:
                return VK_SAMPLE_COUNT_2_BIT;

            case 4:
                return VK_SAMPLE_COUNT_4_BIT;

            case 8:
                return VK_SAMPLE_COUNT_8_BIT;

            case 16:
                return VK_SAMPLE_COUNT_16_BIT;

            case 32:
                return VK_SAMPLE_COUNT_32_BIT;

            case 64:
                return VK_SAMPLE_COUNT_64_BIT;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapSampleCount: Sample count is unknown");
        }
    }

    VkCompareOp Pipeline::reverseMapCompareOperation(CompareOperation compare) {
        switch (compare) {
            case CompareOperation::EQUAL:
                return VK_COMPARE_OP_EQUAL;

            case CompareOperation::NOT_EQUAL:
                return VK_COMPARE_OP_NOT_EQUAL;

            case CompareOperation::GREATER:
                return VK_COMPARE_OP_GREATER;

            case CompareOperation::GREATER_EQUAL:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;

            case CompareOperation::LESS:
                return VK_COMPARE_OP_LESS;

            case CompareOperation::LESS_EQUAL:
                return VK_COMPARE_OP_LESS_OR_EQUAL;

            case CompareOperation::ALWAYS:
                return VK_COMPARE_OP_ALWAYS;

            case CompareOperation::NEVER:
                return VK_COMPARE_OP_NEVER;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapCompareOperation: Compare operation is unknown");
        }
    }

    VkStencilOpState Pipeline::reverseMapStencilOperationState(PerFaceRasterisationState perFaceState) {
        return {
            .failOp = reverseMapStencilOperation(perFaceState.stencilFailOperation),
            .passOp = reverseMapStencilOperation(perFaceState.passOperation),
            .depthFailOp = reverseMapStencilOperation(perFaceState.depthFailOperation),
            .compareOp = reverseMapCompareOperation(perFaceState.stencilComparison),
            .compareMask = perFaceState.stencilCompareMask,
            .writeMask = perFaceState.stencilWriteMask,
            .reference = 0,
        };
    }

    VkBlendFactor Pipeline::reverseMapBlendFactor(BlendFactor factor) {
        switch (factor) {
            case BlendFactor::ZERO:
                return VK_BLEND_FACTOR_ZERO;

            case BlendFactor::ONE:
                return VK_BLEND_FACTOR_ONE;

            case BlendFactor::SOURCE_COLOUR:
                return VK_BLEND_FACTOR_SRC_COLOR;

            case BlendFactor::ONE_MINUS_SOURCE_COLOUR:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

            case BlendFactor::DESTINATION_COLOUR:
                return VK_BLEND_FACTOR_DST_COLOR;

            case BlendFactor::ONE_MINUS_DESTINATION_COLOUR:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;

            case BlendFactor::SOURCE_ALPHA:
                return VK_BLEND_FACTOR_SRC_ALPHA;

            case BlendFactor::ONE_MINUS_SOURCE_ALPHA:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

            case BlendFactor::DESTINATION_ALPHA:
                return VK_BLEND_FACTOR_DST_ALPHA;

            case BlendFactor::ONE_MINUS_DESTINATION_ALPHA:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;

            case BlendFactor::CONSTANT_COLOUR:
                return VK_BLEND_FACTOR_CONSTANT_COLOR;

            case BlendFactor::ONE_MINUS_CONSTANT_COLOUR:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;

            case BlendFactor::CONSTANT_ALPHA:
                return VK_BLEND_FACTOR_CONSTANT_ALPHA;

            case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;

            case BlendFactor::SOURCE_ALPHA_SATURATE:
                return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapBlendFactor: Blend factor is unknown");
        }
    }

    VkBlendOp Pipeline::reverseMapBlendOperation(BlendOperation operation) {
        switch (operation) {
            case BlendOperation::ADD:
                return VK_BLEND_OP_ADD;

            case BlendOperation::SUBTRACT:
                return VK_BLEND_OP_SUBTRACT;

            case BlendOperation::REVERSE_SUBTRACT:
                return VK_BLEND_OP_REVERSE_SUBTRACT;

            case BlendOperation::MIN:
                return VK_BLEND_OP_MIN;

            case BlendOperation::MAX:
                return VK_BLEND_OP_MAX;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapBlendOperation: Blend operation is unknown");
        }
    }

    VkStencilOp Pipeline::reverseMapStencilOperation(ValueOperation operation) {
        switch (operation) {
            case ValueOperation::KEEP:
                return VK_STENCIL_OP_KEEP;

            case ValueOperation::ZERO:
                return VK_STENCIL_OP_ZERO;

            case ValueOperation::REPLACE:
                return VK_STENCIL_OP_REPLACE;

            case ValueOperation::INCREMENT_AND_CLAMP:
                return VK_STENCIL_OP_INCREMENT_AND_CLAMP;

            case ValueOperation::DECREMENT_AND_CLAMP:
                return VK_STENCIL_OP_DECREMENT_AND_CLAMP;

            case ValueOperation::INVERT:
                return VK_STENCIL_OP_INVERT;

            case ValueOperation::INCREMENT_AND_WRAP:
                return VK_STENCIL_OP_INCREMENT_AND_WRAP;

            case ValueOperation::DECREMENT_AND_WRAP:
                return VK_STENCIL_OP_DECREMENT_AND_WRAP;

            default:
                throw std::runtime_error("Call failed: renderer::Pipeline::reverseMapStencilOperation: Stencil operation is unknown");
        }
    }
}