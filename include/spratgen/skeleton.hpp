#pragma once

#include <string>
#include <vector>

#include "silhouette.hpp"

namespace spratgen {

struct Joint {
    std::string name;
    float x = 0.0f;
    float y = 0.0f;
};

struct Skeleton {
    std::vector<Joint> joints;
};

class SkeletonBuilder {
public:
    Skeleton build(const Silhouette& silhouette) const;
};

}  // namespace spratgen