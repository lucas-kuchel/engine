#include <Program/Application.hpp>

#include <exception>
#include <print>

int main(int argc, char** argv)
{
    try
    {
        std::vector<std::string_view> arguments(argv, argv + argc);

        Program::Application application(arguments);

        while (application.ShouldUpdate())
        {
            application.Update();
        }

        return 0;
    }
    catch (std::exception& exception)
    {
        std::println("Runtime exception: {}", exception.what());

        return 1;
    }
}