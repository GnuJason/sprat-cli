#include "pose_interp.hpp"

#include <algorithm>

namespace spratgen {

PoseSkeleton PoseInterpolator::apply(const Skeleton& base, const Skeleton& target, float t) {
    const float clampedT = std::clamp(t, 0.0f, 1.0f);

    PoseSkeleton posed;
    posed.head = lerpJoint(base.head, target.head, clampedT);
    posed.torso = lerpJoint(base.torso, target.torso, clampedT);
    posed.left_arm = lerpJoint(base.left_arm, target.left_arm, clampedT);
    posed.right_arm = lerpJoint(base.right_arm, target.right_arm, clampedT);
    posed.left_leg = lerpJoint(base.left_leg, target.left_leg, clampedT);
    posed.right_leg = lerpJoint(base.right_leg, target.right_leg, clampedT);
    return posed;
}

PoseJoint PoseInterpolator::lerpJoint(const Joint& a, const Joint& b, float t) {
    const float x = static_cast<float>(a.x) + static_cast<float>(b.x - a.x) * t;
    const float y = static_cast<float>(a.y) + static_cast<float>(b.y - a.y) * t;
    return PoseJoint{static_cast<int>(x), static_cast<int>(y)};
}

}  // namespace spratgen