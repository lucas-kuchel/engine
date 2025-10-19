#pragma once

#include <string>

namespace engine::components {
    struct Script {
        std::string function;
        std::string filepath;
    };

    struct Action {
        std::string name;
        Script script;
    };
}