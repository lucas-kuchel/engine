#include <app/program.hpp>

#include <exception>
#include <print>

int main() {
    try {
        app::Program program;

        program.start();

        while (program.shouldUpdate()) {
            program.update();
        }

        program.close();

        return 0;
    }
    catch (const std::exception& exception) {
        std::println("Runtime exception: {}", exception.what());

        return 1;
    }
}