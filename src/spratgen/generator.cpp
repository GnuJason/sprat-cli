#include "generator.hpp"

#include <utility>

namespace spratgen {

Generator::Generator(Config config)
    : config_(std::move(config)) {}

Image Generator::loadMasterFrame() const {
    Image image;
    image.width = 1;
    image.height = 1;
    image.pixels.push_back(Color{});
    return image;
}

std::vector<Color> Generator::setupPalette(const Image& image) const {
    return paletteManager_.quantize(paletteManager_.extractFromImage(image), 16);
}

Skeleton Generator::buildSkeleton(const Image& image) const {
    return skeletonBuilder_.build(silhouetteExtractor_.extract(image));
}

PoseModel Generator::setupPoseModel(const std::string& animType, std::size_t frameCount) const {
    return PoseModel(animType, frameCount);
}

std::vector<Image> Generator::generateFrames(const std::string& animType, std::size_t frameCount) const {
    const Image masterFrame = loadMasterFrame();
    const std::vector<Color> palette = setupPalette(masterFrame);
    const Skeleton baseSkeleton = buildSkeleton(masterFrame);
    const PoseModel poseModel = setupPoseModel(animType, frameCount);

    std::vector<Image> frames;
    frames.reserve(frameCount);
    std::vector<std::string> exportedPaths;
    exportedPaths.reserve(frameCount);

    for (std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        const Skeleton posedSkeleton = poseInterpolator_.apply(baseSkeleton, poseModel, frameIndex);
        frames.push_back(renderer_.renderFrame(posedSkeleton, palette, frameIndex));
        exportedPaths.push_back("frame_" + std::to_string(frameIndex) + ".ppm");
    }

    exporter_.finalizeMetadata(exportedPaths, "spratgen.frames.txt");
    return frames;
}

}  // namespace spratgen