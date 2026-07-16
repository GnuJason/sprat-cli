#pragma once

#include <string>
#include <vector>

namespace spratgen {

class Config {
public:
    std::vector<std::string> args;

    static Config fromArgs(int argc, char** argv);
};

}  // namespace spratgen