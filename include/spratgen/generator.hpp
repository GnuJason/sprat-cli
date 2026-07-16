#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "config.hpp"
#include "export.hpp"
#include "palette.hpp"
#include "pose_interp.hpp"
#include "pose_model.hpp"
#include "renderer.hpp"
#include "silhouette.hpp"
#include "skeleton.hpp"

namespace spratgen {

class Generator {
public:
    explicit Generator(Config config);

    Image loadMasterFrame() const;
    std::vector<Color> setupPalette(const Image& image) const;
    Skeleton buildSkeleton(const Image& image) const;
    PoseModel setupPoseModel(const std::string& animType, std::size_t frameCount) const;
    std::vector<Image> generateFrames(const std::string& animType, std::size_t frameCount) const;

private:
    Config config_;
    PaletteManager paletteManager_;
    SilhouetteExtractor silhouetteExtractor_;
    SkeletonBuilder skeletonBuilder_;
    PoseInterpolator poseInterpolator_;
    PixelRenderer renderer_;
    FrameExporter exporter_;
};

}  // namespace spratgen