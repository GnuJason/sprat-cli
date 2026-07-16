#include "renderer.hpp"

namespace spratgen {

Image PixelRenderer::renderFrame(const Skeleton& posedSkeleton, const std::vector<Color>& palette, std::size_t frameIndex) const {
    Image image;
    image.width = posedSkeleton.joints.empty() ? 1 : static_cast<int>(posedSkeleton.joints.size());
    image.height = 1;
    if (!palette.empty()) {
        image.pixels.push_back(palette[frameIndex % palette.size()]);
    } else {
        image.pixels.push_back(Color{});
    }
    return image;
}

}  // namespace spratgen