#include "generator.hpp"

int main(int argc, char** argv) {
    const spratgen::Config config = spratgen::Config::fromArgs(argc, argv);
    spratgen::Generator generator(config);
    static_cast<void>(generator.generateFrames("idle", 1));
    return 0;
}