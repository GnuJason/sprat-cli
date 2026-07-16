#pragma once

#include "skeleton.hpp"

namespace spratgen {

struct PoseJoint {
    int x = 0;
    int y = 0;
};

struct PoseSkeleton {
    PoseJoint head;
    PoseJoint torso;
    PoseJoint left_arm;
    PoseJoint right_arm;
    PoseJoint left_leg;
    PoseJoint right_leg;
};

class PoseInterpolator {
public:
    PoseSkeleton apply(const Skeleton& base, const Skeleton& target, float t);

private:
    PoseJoint lerpJoint(const Joint& a, const Joint& b, float t);
};

}  // namespace spratgen