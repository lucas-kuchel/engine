#pragma once

#include <data/unique.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/pipeline.hpp>
#include <renderer/resources/shader.hpp>

#include <renderer/commands/buffer.hpp>

namespace app {
    class Program;
}

namespace game {
    class Instance {
    public:
        Instance(app::Program& program);
        ~Instance();

        renderer::RenderPassCreateInfo makeRequiredRenderPass();

        void start();
        void update();
        void close();

        void render();

    private:
        app::Program& program_;

        data::Unique<renderer::PipelineLayout> basicPipelineLayout_;
        data::Unique<renderer::Buffer> vertexBuffer_;
        data::Unique<renderer::Buffer> indexBuffer_;

        std::vector<renderer::Pipeline> pipelines_;

        data::NullableRef<renderer::Pipeline> basicPipeline_;
        data::NullableRef<renderer::DescriptorSet> basicDescriptorSet_;
    };
}