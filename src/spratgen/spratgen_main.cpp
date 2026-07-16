#include "generator.hpp"

#include <algorithm>
#include <iostream>

int main(int argc, char** argv) {
    const spratgen::Config config = spratgen::Config::fromArgs(argc, argv);
    spratgen::Generator generator(config);

    const spratgen::Image masterFrame = generator.loadMasterFrame();
    const spratgen::Silhouette& silhouette = generator.silhouette();
    const auto maskPixels = static_cast<unsigned long long>(std::count(silhouette.mask.begin(), silhouette.mask.end(), static_cast<std::uint8_t>(1)));
    const auto outlinePixels = static_cast<unsigned long long>(std::count(silhouette.outline.begin(), silhouette.outline.end(), static_cast<std::uint8_t>(1)));
    const spratgen::Skeleton& skeleton = generator.buildSkeleton(masterFrame);

    std::cout << "Silhouette: " << silhouette.width << 'x' << silhouette.height << '\n';
    std::cout << "Mask pixels: " << maskPixels << '\n';
    std::cout << "Outline pixels: " << outlinePixels << '\n';
    std::cout << "Head: " << skeleton.head.x << ',' << skeleton.head.y << '\n';
    std::cout << "Torso: " << skeleton.torso.x << ',' << skeleton.torso.y << '\n';
    std::cout << "Left arm: " << skeleton.left_arm.x << ',' << skeleton.left_arm.y << '\n';
    std::cout << "Right arm: " << skeleton.right_arm.x << ',' << skeleton.right_arm.y << '\n';
    std::cout << "Left leg: " << skeleton.left_leg.x << ',' << skeleton.left_leg.y << '\n';
    std::cout << "Right leg: " << skeleton.right_leg.x << ',' << skeleton.right_leg.y << '\n';

    static_cast<void>(generator.generateFrames("idle", 1));
    return 0;
}