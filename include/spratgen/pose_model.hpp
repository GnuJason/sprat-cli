#pragma once

#include <cstddef>
#include <string>

#include "skeleton.hpp"

namespace spratgen {

class PoseModel {
public:
    PoseModel(std::string animType, std::size_t frameCount);

    Skeleton getKeyframe(std::size_t frameIndex) const;

private:
    std::string animType_;
    std::size_t frameCount_ = 0;
};

}  // namespace spratgen