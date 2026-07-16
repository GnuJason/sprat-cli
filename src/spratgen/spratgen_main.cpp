#include "generator.hpp"

#include "pose_interp.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>

namespace {

spratgen::Skeleton offset_skeleton(const spratgen::Skeleton& skeleton, int delta) {
    spratgen::Skeleton target = skeleton;
    target.head.x += delta;
    target.head.y += delta;
    target.torso.x += delta;
    target.torso.y += delta;
    target.left_arm.x += delta;
    target.left_arm.y += delta;
    target.right_arm.x += delta;
    target.right_arm.y += delta;
    target.left_leg.x += delta;
    target.left_leg.y += delta;
    target.right_leg.x += delta;
    target.right_leg.y += delta;
    return target;
}

}  // namespace

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

    spratgen::PoseInterpolator poseInterp;
    const spratgen::Skeleton targetSkeleton = offset_skeleton(skeleton, 10);
    const spratgen::PoseSkeleton posed = poseInterp.apply(skeleton, targetSkeleton, 0.5f);
    std::cout << "Interp head: " << posed.head.x << ',' << posed.head.y << '\n';
    std::cout << "Interp torso: " << posed.torso.x << ',' << posed.torso.y << '\n';
    std::cout << "Interp left arm: " << posed.left_arm.x << ',' << posed.left_arm.y << '\n';
    std::cout << "Interp right arm: " << posed.right_arm.x << ',' << posed.right_arm.y << '\n';
    std::cout << "Interp left leg: " << posed.left_leg.x << ',' << posed.left_leg.y << '\n';
    std::cout << "Interp right leg: " << posed.right_leg.x << ',' << posed.right_leg.y << '\n';

    static_cast<void>(generator.generateFrames("idle", 1));
    return 0;
}