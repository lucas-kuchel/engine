#include <engine/engine.hpp>

#include <exception>
#include <print>

int main() {
    try {
        engine::Engine engine;

        engine.run();

        return 0;
    }
    catch (const std::exception& exception) {
        std::println("Runtime exception: {}", exception.what());

        return 1;
    }
}