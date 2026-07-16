#include "generator.hpp"

#include <stb_image.h>

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace spratgen {

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

std::vector<Color> Generator::setupPalette(const Image& image) const {
    return paletteManager_.quantize(paletteManager_.extractFromImage(image), 16);
}

Skeleton Generator::buildSkeleton(const Image& image) {
    if (silhouette_.mask.empty() || silhouette_.width != image.width || silhouette_.height != image.height) {
        silhouette_ = silhouetteExtractor_.extract(image);
    }
    skeleton_ = skeletonBuilder_.build(silhouette_);
    return skeleton_;
}

PoseModel Generator::setupPoseModel(const std::string& animType, std::size_t frameCount) const {
    return PoseModel(animType, frameCount);
}

std::vector<Image> Generator::generateFrames(const std::string& animType, std::size_t frameCount) {
    Image image;
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

const Silhouette& Generator::silhouette() const {
    return silhouette_;
}

const Skeleton& Generator::skeleton() const {
    return skeleton_;
}

}  // namespace spratgen