#include "generator.hpp"

#include <stb_image.h>

#include <cstddef>
#include <algorithm>
#include <utility>

namespace spratgen {

namespace {

bool has_named_pose(const Skeleton& skeleton) {
    return skeleton.head.name == "head"
        || skeleton.torso.name == "torso"
        || skeleton.left_arm.name == "left_arm"
        || skeleton.right_arm.name == "right_arm"
        || skeleton.left_leg.name == "left_leg"
        || skeleton.right_leg.name == "right_leg";
}

Skeleton make_render_skeleton(const PoseSkeleton& posed) {
    Skeleton skeleton;
    skeleton.head = Joint("head", posed.head.x, posed.head.y);
    skeleton.torso = Joint("torso", posed.torso.x, posed.torso.y);
    skeleton.left_arm = Joint("left_arm", posed.left_arm.x, posed.left_arm.y);
    skeleton.right_arm = Joint("right_arm", posed.right_arm.x, posed.right_arm.y);
    skeleton.left_leg = Joint("left_leg", posed.left_leg.x, posed.left_leg.y);
    skeleton.right_leg = Joint("right_leg", posed.right_leg.x, posed.right_leg.y);
    skeleton.joints = {
        skeleton.head,
        skeleton.torso,
        skeleton.left_arm,
        skeleton.right_arm,
        skeleton.left_leg,
        skeleton.right_leg,
    };
    return skeleton;
}

Skeleton make_offset_skeleton(const Skeleton& skeleton, int delta) {
    Skeleton target = skeleton;
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
    target.joints = {
        target.head,
        target.torso,
        target.left_arm,
        target.right_arm,
        target.left_leg,
        target.right_leg,
    };
    return target;
}

}  // namespace

Generator::Generator(Config config)
    : config_(std::move(config)) {}

Image Generator::loadMasterFrame() {
    Image image;
    if (config_.args.size() > 1) {
        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* rawPixels = stbi_load(config_.args[1].c_str(), &width, &height, &channels, 4);
        if (rawPixels != nullptr) {
            image.width = width;
            image.height = height;
            const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
            image.pixels.reserve(pixelCount);
            for (std::size_t index = 0; index < pixelCount; ++index) {
                const std::size_t base = index * 4U;
                image.pixels.push_back(Color{
                    rawPixels[base],
                    rawPixels[base + 1U],
                    rawPixels[base + 2U],
                    rawPixels[base + 3U],
                });
            }
            stbi_image_free(rawPixels);
        }
    }

    if (image.pixels.empty()) {
        image.width = 1;
        image.height = 1;
        image.pixels.push_back(Color{});
    }

    silhouette_ = silhouetteExtractor_.extract(image);
    return image;
}

Palette Generator::setupPalette(const Image& image) const {
    const std::vector<Color> colors = paletteManager_.quantize(paletteManager_.extractFromImage(image), 16);
    Palette palette;
    if (!colors.empty()) {
        palette.base = colors.front();
        palette.accent = colors.size() > 1 ? colors[1] : colors.front();
    }
    return palette;
}

Skeleton Generator::buildSkeleton(const Image& image) {
    if (silhouette_.mask.empty() || silhouette_.width != image.width || silhouette_.height != image.height) {
        silhouette_ = silhouetteExtractor_.extract(image);
    }
    skeleton_ = skeletonBuilder_.build(silhouette_);
    return skeleton_;
}

PoseModel Generator::setupPoseModel(const std::string& animType, std::size_t frameCount) {
    if (!has_named_pose(skeleton_)) {
        Image image;
        image.width = silhouette_.width;
        image.height = silhouette_.height;
        if (!silhouette_.mask.empty()) {
            buildSkeleton(image);
        }
    }
    return PoseModel(animType, frameCount);
}

std::vector<RenderedFrame> Generator::generateFrames(const std::string& animType, std::size_t frameCount) {
    const Image masterFrame = loadMasterFrame();
    const Palette palette = setupPalette(masterFrame);
    const Skeleton inferredSkeleton = buildSkeleton(masterFrame);
    const PoseModel poseModel = setupPoseModel(animType, frameCount);
    Skeleton baseSkeleton = poseModel.getKeyframe(0);
    Skeleton targetSkeleton = poseModel.getKeyframe(1);

    if (!has_named_pose(baseSkeleton)) {
        baseSkeleton = inferredSkeleton;
    }
    if (!has_named_pose(targetSkeleton)) {
        targetSkeleton = make_offset_skeleton(inferredSkeleton, 10);
    }

    std::vector<RenderedFrame> frames;
    frames.reserve(frameCount);
    std::vector<std::string> exportedPaths;
    exportedPaths.reserve(frameCount);

    for (std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        const float t = frameCount > 1
            ? static_cast<float>(frameIndex) / static_cast<float>(frameCount - 1)
            : 0.0f;
        const PoseSkeleton posed = poseInterp_.apply(baseSkeleton, targetSkeleton, t);
        frames.push_back(renderer_.renderFrame(silhouette_, posed, palette));
        exportedPaths.push_back("frame_" + std::to_string(frameIndex) + ".ppm");
    }

    exporter_.finalizeMetadata(exportedPaths, "spratgen.frames.txt");
    return frames;
}

const Silhouette& Generator::silhouette() const {
    return silhouette_;
}

const Skeleton& Generator::skeleton() const {
    return skeleton_;
}

}  // namespace spratgen