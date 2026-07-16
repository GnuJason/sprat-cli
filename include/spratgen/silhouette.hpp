#pragma once

#include <vector>

#include "palette.hpp"

namespace spratgen {

struct Silhouette {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> mask;
};

class SilhouetteExtractor {
public:
    Silhouette extract(const Image& image) const;
};

}  // namespace spratgen