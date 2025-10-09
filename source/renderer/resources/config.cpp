#include <renderer/resources/config.hpp>

namespace renderer {
    VkFlags BufferUsageFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {BufferUsageFlags::VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
            {BufferUsageFlags::INDEX, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
            {BufferUsageFlags::STORAGE, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
            {BufferUsageFlags::TRANSFER_DESTINATION, VK_BUFFER_USAGE_TRANSFER_DST_BIT},
            {BufferUsageFlags::TRANSFER_SOURCE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT},
            {BufferUsageFlags::UNIFORM, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }

    VkFlags StencilFaceFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {StencilFaceFlags::BACK, VK_STENCIL_FACE_BACK_BIT},
            {StencilFaceFlags::FRONT, VK_STENCIL_FACE_BACK_BIT},
            {StencilFaceFlags::BOTH, VK_STENCIL_FACE_FRONT_AND_BACK},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }

    VkFlags ImageAspectFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {ImageAspectFlags::COLOUR, VK_IMAGE_ASPECT_COLOR_BIT},
            {ImageAspectFlags::DEPTH, VK_IMAGE_ASPECT_DEPTH_BIT},
            {ImageAspectFlags::STENCIL, VK_IMAGE_ASPECT_STENCIL_BIT},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }

    VkFlags AccessFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {AccessFlags::INDIRECT_COMMAND_READ, VK_ACCESS_INDIRECT_COMMAND_READ_BIT},
            {AccessFlags::INDEX_READ, VK_ACCESS_INDEX_READ_BIT},
            {AccessFlags::VERTEX_ATTRIBUTE_READ, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT},
            {AccessFlags::UNIFORM_READ, VK_ACCESS_UNIFORM_READ_BIT},
            {AccessFlags::INPUT_ATTACHMENT_READ, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT},
            {AccessFlags::SHADER_READ, VK_ACCESS_SHADER_READ_BIT},
            {AccessFlags::SHADER_WRITE, VK_ACCESS_SHADER_WRITE_BIT},
            {AccessFlags::COLOR_ATTACHMENT_READ, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT},
            {AccessFlags::COLOR_ATTACHMENT_WRITE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
            {AccessFlags::DEPTH_STENCIL_ATTACHMENT_READ, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT},
            {AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT},
            {AccessFlags::TRANSFER_READ, VK_ACCESS_TRANSFER_READ_BIT},
            {AccessFlags::TRANSFER_WRITE, VK_ACCESS_TRANSFER_WRITE_BIT},
            {AccessFlags::HOST_READ, VK_ACCESS_HOST_READ_BIT},
            {AccessFlags::HOST_WRITE, VK_ACCESS_HOST_WRITE_BIT},
            {AccessFlags::MEMORY_READ, VK_ACCESS_MEMORY_READ_BIT},
            {AccessFlags::MEMORY_WRITE, VK_ACCESS_MEMORY_WRITE_BIT},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }

    VkFlags FenceCreateFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {FenceCreateFlags::START_SIGNALED, VK_FENCE_CREATE_SIGNALED_BIT},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }

    VkFlags ImageUsageFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {ImageUsageFlags::COLOR_ATTACHMENT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
            {ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
            {ImageUsageFlags::SAMPLED, VK_IMAGE_USAGE_SAMPLED_BIT},
            {ImageUsageFlags::STORAGE, VK_IMAGE_USAGE_STORAGE_BIT},
            {ImageUsageFlags::TRANSFER_DESTINATION, VK_IMAGE_USAGE_TRANSFER_DST_BIT},
            {ImageUsageFlags::TRANSFER_SOURCE, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }

    VkFlags PipelineStageFlags::mapFrom(Flags flags) {
        struct FlagMap {
            uint32_t flag;
            VkFlags vkFlag;
        };

        constexpr FlagMap flagMapping[] = {
            {PipelineStageFlags::TOP_OF_PIPE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {PipelineStageFlags::DRAW_INDIRECT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT},
            {PipelineStageFlags::VERTEX_INPUT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT},
            {PipelineStageFlags::VERTEX_SHADER, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT},
            {PipelineStageFlags::FRAGMENT_SHADER, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
            {PipelineStageFlags::EARLY_FRAGMENT_TESTS, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
            {PipelineStageFlags::LATE_FRAGMENT_TESTS, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT},
            {PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            {PipelineStageFlags::TRANSFER, VK_PIPELINE_STAGE_TRANSFER_BIT},
            {PipelineStageFlags::BOTTOM_OF_PIPE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT},
            {PipelineStageFlags::HOST, VK_PIPELINE_STAGE_HOST_BIT},
            {PipelineStageFlags::ALL_GRAPHICS, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT},
            {PipelineStageFlags::ALL_COMMANDS, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT},
        };

        VkFlags vkFlags = 0;

        for (auto& flag : flagMapping) {
            if (flags & flag.flag) {
                vkFlags |= flag.vkFlag;
            }
        }

        return vkFlags;
    }
}