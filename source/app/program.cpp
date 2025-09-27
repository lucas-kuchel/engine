#include <app/program.hpp>

namespace app {
    Program::Program() {
    }

    void Program::start() {
        WindowCreateInfo windowCreateInfo = {
            .context = context_,
            .extent = {1280, 720},
            .title = "Engine",
            .visibility = WindowVisibility::WINDOWED,
            .resizable = false,
        };

        window_.create(windowCreateInfo);

        renderer::SurfaceCreateInfo surfaceCreateInfo = {
            .instance = instance_,
            .window = window_,
        };

        surface_.create(surfaceCreateInfo);

        renderer::QueueCreateInfo renderQueueCreateInfo = {
            .type = renderer::QueueType::RENDER,
            .instance = instance_,
            .surface = surface_,
        };

        renderQueue_.create(renderQueueCreateInfo);

        renderer::QueueCreateInfo presentQueueCreateInfo = {
            .type = renderer::QueueType::PRESENT,
            .instance = instance_,
            .surface = surface_,
        };

        presentQueue_.create(presentQueueCreateInfo);

        renderer::DeviceCreateInfo deviceCreateInfo = {
            .instance = instance_,
            .queues = {renderQueue_, presentQueue_},
        };

        device_.create(deviceCreateInfo);

        renderer::SwapchainCreateInfo swapchainCreateInfo = {
            .instance = instance_,
            .surface = surface_,
            .device = device_,
            .imageCount = 3,
            .synchronise = true,
        };

        swapchain_.create(swapchainCreateInfo);
    }

    void Program::update() {
        context_.pollEvents();

        std::queue<WindowEvent>& windowEvents = window_.queryEvents();

        while (!windowEvents.empty()) {
            auto& event = windowEvents.front();

            switch (event.type) {
                case WindowEventType::CLOSED:
                    running_ = false;

                default:
                    break;
            }

            windowEvents.pop();
        }
    }

    void Program::close() {
    }

    bool Program::shouldUpdate() {
        return running_;
    }
}