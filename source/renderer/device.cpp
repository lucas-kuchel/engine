#include <renderer/configuration.hpp>
#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/instance.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/queue.hpp>
#include <renderer/render_pass.hpp>
#include <renderer/shader_module.hpp>
#include <renderer/surface.hpp>

#include <stdexcept>
#include <unordered_map>

namespace renderer {
    Device Device::create(const DeviceCreateInfo& createInfo) {
        Device device;

        std::vector<std::uint32_t> familyIndexMappings;
        std::vector<std::vector<float>> familyIndexPriorities;

        device.queues_.reserve(createInfo.queues.size());

        for (auto& queueCreateInfo : createInfo.queues) {
            device.queues_.push_back(Queue());

            auto& queue = device.queues_.back();

            VkQueueFlags queueTypeNeeded = 0;
            bool isPresentType = false;

            if (queueCreateInfo.flags & QueueFlags::COMPUTE) {
                queueTypeNeeded |= VK_QUEUE_COMPUTE_BIT;
            }

            if (queueCreateInfo.flags & QueueFlags::GRAPHICS) {
                queueTypeNeeded |= VK_QUEUE_GRAPHICS_BIT;
            }

            if (queueCreateInfo.flags & QueueFlags::TRANSFER) {
                queueTypeNeeded |= VK_QUEUE_TRANSFER_BIT;
            }

            if (queueCreateInfo.flags & QueueFlags::PRESENT) {
                isPresentType = true;
            }

            auto& queueFamilyProperties = createInfo.instance.queueFamilyProperties_;
            auto& queueFamilyOccupations = createInfo.instance.queueFamilyOccupations_;

            bool foundQueue = false;

            for (std::uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
                const auto& family = queueFamilyProperties[i];
                VkBool32 presentSupported = VK_FALSE;

                if (isPresentType) {
                    if (queueCreateInfo.surface == nullptr) {
                        throw std::runtime_error("Construction failed: renderer::Queue inside renderer::Device: Present queues require a surface to be created");
                    }

                    auto& surface = queueCreateInfo.surface->surface_;

                    if (vkGetPhysicalDeviceSurfaceSupportKHR(createInfo.instance.physicalDevice_, i, surface, &presentSupported) != VK_SUCCESS) {
                        throw std::runtime_error("Construction failed: renderer::Queue inside renderer::Device: Failed to query surface presentation support");
                    }

                    if (!presentSupported) {
                        continue;
                    }
                }

                bool supportsType = (family.queueFlags & static_cast<std::uint32_t>(queueTypeNeeded)) != 0;

                if (!supportsType && !presentSupported) {
                    continue;
                }

                if (queueFamilyOccupations[i] >= family.queueCount) {
                    continue;
                }

                queue.familyIndex_ = i;
                queue.queueIndex_ = queueFamilyOccupations[i]++;
                foundQueue = true;

                break;
            }

            if (!foundQueue) {
                throw std::runtime_error("Construction failed: renderer::Queue inside renderer::Device: Failed to find queue family for requested queue");
            }

            if (familyIndexMappings.size() <= queue.familyIndex_) {
                familyIndexMappings.resize(queue.familyIndex_ + 1, 0);
                familyIndexPriorities.resize(queue.familyIndex_ + 1);
            }

            if (familyIndexMappings[queue.familyIndex_] < queueFamilyProperties[queue.familyIndex_].queueCount) {
                familyIndexMappings[queue.familyIndex_]++;
                familyIndexPriorities[queue.familyIndex_].push_back(1.0f);
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (std::uint32_t i = 0; i < familyIndexMappings.size(); i++) {
            if (familyIndexMappings[i] == 0 || familyIndexPriorities[i].size() == 0) {
                continue;
            }

            VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = i,
                .queueCount = familyIndexMappings[i],
                .pQueuePriorities = familyIndexPriorities[i].data(),
            };

            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::uint32_t extensionCount = 0;

        if (vkEnumerateDeviceExtensionProperties(createInfo.instance.physicalDevice_, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Device: Failed to enumerate device extensions");
        }

        std::vector<VkExtensionProperties> extensionProperties(extensionCount);

        if (vkEnumerateDeviceExtensionProperties(createInfo.instance.physicalDevice_, nullptr, &extensionCount, extensionProperties.data()) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Device: Failed to enumerate device extensions");
        }

        std::vector<const char*> selectedExtensions;

        bool foundSwapchain = false;

        for (auto& extensionInfo : extensionProperties) {
            bool match = false;

            match |= std::string_view(extensionInfo.extensionName) == "VK_KHR_swapchain";

            if (match) {
                foundSwapchain = true;

                selectedExtensions.push_back(extensionInfo.extensionName);
            }
        }

        if (!foundSwapchain) {
            throw std::runtime_error("Construction failed: renderer::Device: Swapchain is unsupported on this system");
        }

        std::uint32_t extensionInfoCount = static_cast<std::uint32_t>(selectedExtensions.size());
        std::uint32_t queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = queueCreateInfoCount,
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = extensionInfoCount,
            .ppEnabledExtensionNames = selectedExtensions.data(),
            .pEnabledFeatures = nullptr,
        };

        if (vkCreateDevice(createInfo.instance.physicalDevice_, &deviceCreateInfo, nullptr, &device.device_) != VK_SUCCESS) {
            device.device_ = nullptr;
            device.allocator_ = nullptr;

            return device;
        }

        device.instance_ = &createInfo.instance;

        for (auto& queue : device.queues_) {
            vkGetDeviceQueue(device.device_, queue.familyIndex_, queue.queueIndex_, &queue.queue_);
        }

        VmaAllocatorCreateInfo allocatorCreateInfo = {
            .flags = 0,
            .physicalDevice = device.instance_->physicalDevice_,
            .device = device.device_,
            .preferredLargeHeapBlockSize = 0,
            .pAllocationCallbacks = nullptr,
            .pDeviceMemoryCallbacks = nullptr,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = nullptr,
            .instance = device.instance_->instance_,
            .vulkanApiVersion = device.instance_->apiVersion_,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };

        if (vmaCreateAllocator(&allocatorCreateInfo, &device.allocator_) != VK_SUCCESS) {
            vkDestroyDevice(device.device_, nullptr);
            device.device_ = nullptr;
            device.allocator_ = nullptr;
        }

        return device;
    }

    void Device::destroy(Device& device) {
        if (device.allocator_) {
            vmaDestroyAllocator(device.allocator_);

            device.allocator_ = nullptr;
        }

        if (device.device_) {
            vkDestroyDevice(device.device_, nullptr);

            device.device_ = nullptr;
        }
    }

    bool Device::waitIdle(Device& device) {
        return vkDeviceWaitIdle(device.device_) == VK_SUCCESS;
    }

    bool Device::waitForFences(Device& device, const std::vector<Fence>& fences, bool waitAll, std::uint32_t timeout) {
        std::vector<VkFence> vkFences(fences.size());

        for (std::size_t i = 0; i < fences.size(); i++) {
            vkFences[i] = fences[i].fence_;
        }

        return vkWaitForFences(device.device_, static_cast<std::uint32_t>(vkFences.size()), vkFences.data(), waitAll, timeout) == VK_SUCCESS;
    }

    bool Device::resetFences(Device& device, const std::vector<Fence>& fences) {
        std::vector<VkFence> vkFences(fences.size());

        for (std::size_t i = 0; i < fences.size(); i++) {
            vkFences[i] = fences[i].fence_;
        }

        return vkResetFences(device.device_, static_cast<std::uint32_t>(vkFences.size()), vkFences.data()) == VK_SUCCESS;
    }

    std::vector<Pipeline> Device::createPipelines(Device& device, const std::vector<PipelineCreateInfo>& createInfos) {
        struct PipelineCreationData {
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
            std::vector<VkVertexInputBindingDescription> bindings;
            std::vector<VkVertexInputAttributeDescription> attributes;
            std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

            VkPipelineVertexInputStateCreateInfo vertexInput;
            VkPipelineInputAssemblyStateCreateInfo inputAssembly;
            VkPipelineViewportStateCreateInfo viewportState;
            VkPipelineRasterizationStateCreateInfo raster;
            VkPipelineMultisampleStateCreateInfo multisample;
            VkPipelineDepthStencilStateCreateInfo depthStencil;
            VkPipelineColorBlendStateCreateInfo colourBlend;
        };

        std::vector<VkGraphicsPipelineCreateInfo> pipelineCreateInfos(createInfos.size());
        std::vector<VkPipeline> pipelineHandles(createInfos.size(), VK_NULL_HANDLE);
        std::vector<PipelineCreationData> creationData(createInfos.size());
        std::vector<Pipeline> pipelines;

        pipelines.reserve(createInfos.size());

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
            VK_DYNAMIC_STATE_DEPTH_BIAS,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            VK_DYNAMIC_STATE_DEPTH_BOUNDS,
            VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
            VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data(),
        };

        for (std::size_t i = 0; i < createInfos.size(); i++) {
            auto& createInfo = createInfos[i];
            auto& createData = creationData[i];
            auto& pipelineCreateInfo = pipelineCreateInfos[i];

            createData.shaderStages.resize(createInfo.shaderStages.size());
            createData.bindings.resize(createInfo.vertexInput.bindings.size());
            createData.attributes.resize(createInfo.vertexInput.attributes.size());
            createData.blendAttachments.resize(createInfo.colourBlend.attachments.size());

            for (std::size_t j = 0; j < createData.shaderStages.size(); j++) {
                auto& info = createData.shaderStages[j];
                auto& stage = createInfo.shaderStages[j];

                info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = Pipeline::reverseMapShaderStage(stage.stage),
                    .module = stage.module.module_,
                    .pName = stage.entry.c_str(),
                    .pSpecializationInfo = nullptr,
                };
            }

            for (std::size_t j = 0; j < createData.bindings.size(); j++) {
                auto& description = createData.bindings[j];
                auto& binding = createInfo.vertexInput.bindings[j];

                description = {
                    .binding = binding.binding,
                    .stride = binding.strideBytes,
                    .inputRate = Pipeline::reverseMapVertexInputRate(binding.inputRate),
                };
            }

            std::unordered_map<std::uint32_t, std::uint32_t> bindingOffsets;

            for (std::size_t j = 0; j < createData.attributes.size(); j++) {
                auto& description = createData.attributes[j];
                auto& attribute = createInfo.vertexInput.attributes[j];

                std::uint32_t& offset = bindingOffsets[attribute.binding];

                description = {
                    .location = attribute.location,
                    .binding = attribute.binding,
                    .format = Pipeline::reverseMapVertexAttributeFormat(attribute.format),
                    .offset = offset,
                };

                switch (attribute.format) {
                    case VertexAttributeFormat::R32_FLOAT:
                    case VertexAttributeFormat::R32_INT:
                    case VertexAttributeFormat::R32_UINT:
                        offset += 4;
                        break;

                    case VertexAttributeFormat::R32G32_FLOAT:
                    case VertexAttributeFormat::R32G32_INT:
                    case VertexAttributeFormat::R32G32_UINT:
                        offset += 8;
                        break;

                    case VertexAttributeFormat::R32G32B32_FLOAT:
                    case VertexAttributeFormat::R32G32B32_INT:
                    case VertexAttributeFormat::R32G32B32_UINT:
                        offset += 12;
                        break;

                    case VertexAttributeFormat::R32G32B32A32_FLOAT:
                    case VertexAttributeFormat::R32G32B32A32_INT:
                    case VertexAttributeFormat::R32G32B32A32_UINT:
                        offset += 16;
                        break;
                }
            }

            for (std::size_t j = 0; j < createData.blendAttachments.size(); j++) {
                auto& state = createData.blendAttachments[j];
                auto& attachment = createInfo.colourBlend.attachments[j];

                state = {
                    .blendEnable = attachment.blendEnable ? VK_TRUE : VK_FALSE,
                    .srcColorBlendFactor = Pipeline::reverseMapBlendFactor(attachment.sourceColourBlendFactor),
                    .dstColorBlendFactor = Pipeline::reverseMapBlendFactor(attachment.destinationColourBlendFactor),
                    .colorBlendOp = Pipeline::reverseMapBlendOperation(attachment.colourBlendOperation),
                    .srcAlphaBlendFactor = Pipeline::reverseMapBlendFactor(attachment.sourceAlphaBlendFactor),
                    .dstAlphaBlendFactor = Pipeline::reverseMapBlendFactor(attachment.destinationAlphaBlendFactor),
                    .alphaBlendOp = Pipeline::reverseMapBlendOperation(attachment.alphaBlendOperation),
                    .colorWriteMask = 0xF,
                };
            }

            createData.vertexInput = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .vertexBindingDescriptionCount = static_cast<std::uint32_t>(createData.bindings.size()),
                .pVertexBindingDescriptions = createData.bindings.data(),
                .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(createData.attributes.size()),
                .pVertexAttributeDescriptions = createData.attributes.data(),
            };

            createData.inputAssembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .topology = Pipeline::reverseMapPrimitive(createInfo.inputAssembly.topology),
                .primitiveRestartEnable = createInfo.inputAssembly.primitiveRestart ? VK_TRUE : VK_FALSE,
            };

            createData.viewportState = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .viewportCount = createInfo.viewportCount,
                .pViewports = nullptr,
                .scissorCount = createInfo.scissorCount,
                .pScissors = nullptr,
            };

            createData.raster = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthClampEnable = createInfo.rasterisation.depthClampEnable ? VK_TRUE : VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = Pipeline::reverseMapCullMode(createInfo.rasterisation.cullMode),
                .frontFace = Pipeline::reverseMapFrontFace(createInfo.rasterisation.frontFaceWinding),
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0,
                .depthBiasClamp = 0,
                .depthBiasSlopeFactor = 0,
                .lineWidth = 1.0f,
            };

            createData.multisample = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = Pipeline::reverseMapSampleCount(createInfo.multisample.sampleCount),
                .sampleShadingEnable = createInfo.multisample.sampleShadingEnable ? VK_TRUE : VK_FALSE,
                .minSampleShading = createInfo.multisample.minSampleShading,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = createInfo.multisample.alphaToCoverageEnable ? VK_TRUE : VK_FALSE,
                .alphaToOneEnable = createInfo.multisample.alphaToOneEnable ? VK_TRUE : VK_FALSE,
            };

            createData.depthStencil = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthTestEnable = createInfo.rasterisation.depthTestEnable ? VK_TRUE : VK_FALSE,
                .depthWriteEnable = createInfo.rasterisation.depthWriteEnable ? VK_TRUE : VK_FALSE,
                .depthCompareOp = Pipeline::reverseMapCompareOperation(createInfo.rasterisation.frontface.depthComparison),
                .depthBoundsTestEnable = createInfo.rasterisation.depthBoundsTestEnable ? VK_TRUE : VK_FALSE,
                .stencilTestEnable = createInfo.rasterisation.stencilTestEnable ? VK_TRUE : VK_FALSE,
                .front = Pipeline::reverseMapStencilOperationState(createInfo.rasterisation.frontface),
                .back = Pipeline::reverseMapStencilOperationState(createInfo.rasterisation.backface),
                .minDepthBounds = 0.0,
                .maxDepthBounds = 1.0,
            };

            createData.colourBlend = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_NO_OP,
                .attachmentCount = static_cast<std::uint32_t>(createData.blendAttachments.size()),
                .pAttachments = createData.blendAttachments.data(),
                .blendConstants = {},
            };

            pipelineCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stageCount = static_cast<std::uint32_t>(createData.shaderStages.size()),
                .pStages = createData.shaderStages.data(),
                .pVertexInputState = &createData.vertexInput,
                .pInputAssemblyState = &createData.inputAssembly,
                .pTessellationState = nullptr,
                .pViewportState = &createData.viewportState,
                .pRasterizationState = &createData.raster,
                .pMultisampleState = &createData.multisample,
                .pDepthStencilState = &createData.depthStencil,
                .pColorBlendState = &createData.colourBlend,
                .pDynamicState = &dynamicStateInfo,
                .layout = createInfo.layout.pipelineLayout_,
                .renderPass = createInfo.renderPass.renderPass_,
                .subpass = createInfo.subpassIndex,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = 0,
            };
        }

        if (vkCreateGraphicsPipelines(device.device_, VK_NULL_HANDLE, static_cast<std::uint32_t>(pipelineCreateInfos.size()), pipelineCreateInfos.data(), nullptr, pipelineHandles.data()) != VK_SUCCESS) {
            return {};
        }

        for (auto& vkPipeline : pipelineHandles) {
            pipelines.push_back(Pipeline());

            auto& pipeline = pipelines.back();

            pipeline.pipeline_ = vkPipeline;
            pipeline.device_ = &device;
        }

        return pipelines;
    }

    std::span<Queue> Device::getQueues(Device& device) {
        return device.queues_;
    }
}