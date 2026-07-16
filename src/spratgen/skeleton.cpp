#include "skeleton.hpp"

namespace spratgen {

Skeleton SkeletonBuilder::build(const Silhouette& silhouette) const {
    Skeleton skeleton;
    if (silhouette.width > 0 || silhouette.height > 0) {
        skeleton.joints.push_back({"root", silhouette.width / 2.0f, silhouette.height / 2.0f});
    }
    return skeleton;
}

}  // namespace spratgen