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