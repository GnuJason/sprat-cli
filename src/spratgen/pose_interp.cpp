#include "pose_interp.hpp"

namespace spratgen {

Skeleton PoseInterpolator::apply(const Skeleton& skeleton, const PoseModel& poseModel, std::size_t frameIndex) const {
    Skeleton posed = skeleton;
    Skeleton keyframe = poseModel.getKeyframe(frameIndex);
    if (!keyframe.joints.empty()) {
        posed = keyframe;
    }
    return posed;
}

}  // namespace spratgen