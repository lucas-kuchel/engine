#include <app/window.hpp>

#include <GLFW/glfw3.h>

namespace app {
    class GLFWWindowImplementation : public Window::Implementation {
    public:
        GLFWWindowImplementation(const WindowCreateInfo& createInfo);
        ~GLFWWindowImplementation() override;

        void setExtent(const data::Extent2D& extent) override;
        void setTitle(const std::string& title) override;
        void setVisibility(const WindowVisibility& visibility) override;

        [[nodiscard]] const data::Extent2D& getExtent() const override;
        [[nodiscard]] const std::string& getTitle() const override;
        [[nodiscard]] const WindowVisibility& getVisibility() const override;
        [[nodiscard]] std::queue<WindowEvent>& queryEvents() override;

        void* getNativeHandle() override;

    private:
        data::Extent2D extent_;

        std::string title_;
        std::queue<WindowEvent> events_;

        WindowVisibility visibility_;

        GLFWwindow* handle_;

        static void resizeCallback(GLFWwindow* window, int width, int height);
        static void closeCallback(GLFWwindow* window);
        static void focusCallback(GLFWwindow* window, int focused);
        static void iconifyCallback(GLFWwindow* window, int iconified);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int);
        static void mousePositionCallback(GLFWwindow* window, double x, double y);
        static void mouseScrollCallback(GLFWwindow* window, double x, double y);

        [[nodiscard]] static Key mapKey(int key);
        [[nodiscard]] static MouseButton mapMouseButton(int key);
    };
}