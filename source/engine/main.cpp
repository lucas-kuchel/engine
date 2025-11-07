#include <engine/engine.hpp>

#include <print>

int main() {
    try {
        engine::Engine engine;

        engine.run();

        return 0;
    }
    catch (const std::exception& error) {
        std::println("Runtime error: {}", error.what());
    }
}