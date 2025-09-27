#pragma once

#include <app/context.hpp>

namespace app {
    class GLFWContextImplementation : public Context::Implementation {
    public:
        GLFWContextImplementation();
        ~GLFWContextImplementation() override;

        void pollEvents() override;
        void awaitEvents() override;
        void awaitEventsTimeout(double timeout) override;

        ContextBackend queryBackend() const override;

    private:
        bool initialised_ = false;
    };
}