#pragma once

#include <cstddef>

#include "pose_model.hpp"

namespace spratgen {

class PoseInterpolator {
public:
    Skeleton apply(const Skeleton& skeleton, const PoseModel& poseModel, std::size_t frameIndex) const;
};

}  // namespace spratgen