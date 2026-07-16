#pragma once

#include <cstddef>
#include <vector>

#include "palette.hpp"
#include "skeleton.hpp"

namespace spratgen {

class PixelRenderer {
public:
    Image renderFrame(const Skeleton& posedSkeleton, const std::vector<Color>& palette, std::size_t frameIndex) const;
};

}  // namespace spratgen