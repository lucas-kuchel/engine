#pragma once

#include <string>
#include <vector>

namespace world {
    struct Script {
        std::string entry;
        std::string filepath;
    };

    struct Action {
        std::string name;

        std::vector<std::string> parameters;

        Script script;
    };
}