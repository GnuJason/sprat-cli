#include "generator.hpp"

#include <algorithm>
#include <iostream>

int main(int argc, char** argv) {
    const spratgen::Config config = spratgen::Config::fromArgs(argc, argv);
    spratgen::Generator generator(config);

    static_cast<void>(generator.loadMasterFrame());
    const spratgen::Silhouette& silhouette = generator.silhouette();
    const auto maskPixels = static_cast<unsigned long long>(std::count(silhouette.mask.begin(), silhouette.mask.end(), static_cast<std::uint8_t>(1)));
    const auto outlinePixels = static_cast<unsigned long long>(std::count(silhouette.outline.begin(), silhouette.outline.end(), static_cast<std::uint8_t>(1)));

    std::cout << "Silhouette: " << silhouette.width << 'x' << silhouette.height << '\n';
    std::cout << "Mask pixels: " << maskPixels << '\n';
    std::cout << "Outline pixels: " << outlinePixels << '\n';

    static_cast<void>(generator.generateFrames("idle", 1));
    return 0;
}