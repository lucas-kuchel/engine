#include <app/program.hpp>

#include <exception>
#include <print>

int main() {
    try {
        app::Program program;

        return 0;
    }
    catch (const std::exception& exception) {
        std::println("Runtime exception: {}", exception.what());

        return 1;
    }
}