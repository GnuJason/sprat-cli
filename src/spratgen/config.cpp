#include "config.hpp"

#include <utility>

namespace spratgen {

Config Config::fromArgs(int argc, char** argv) {
    Config config;
    config.args.reserve(argc > 0 ? static_cast<std::size_t>(argc) : 0U);
    for (int index = 0; index < argc; ++index) {
        config.args.emplace_back(argv[index] != nullptr ? argv[index] : "");
    }
    return config;
}

}  // namespace spratgen