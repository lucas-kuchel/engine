#pragma once

#include <string>

namespace components {
    struct Script {
        std::string function;
        std::string filepath;
    };

    struct Action {
        std::string name;
        Script script;

        float duration = 0.0f;
        float elapsed = 0.0f;
    };
}