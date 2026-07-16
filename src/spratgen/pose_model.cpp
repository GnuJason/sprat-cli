#include "pose_model.hpp"

#include <utility>

namespace spratgen {

PoseModel::PoseModel(std::string animType, std::size_t frameCount)
    : animType_(std::move(animType)), frameCount_(frameCount) {}

Skeleton PoseModel::getKeyframe(std::size_t frameIndex) const {
    Skeleton skeleton;
    if (frameCount_ == 0) {
        return skeleton;
    }

    skeleton.joints.push_back({animType_.empty() ? "root" : animType_, static_cast<float>(frameIndex), static_cast<float>(frameCount_)});
    return skeleton;
}

}  // namespace spratgen