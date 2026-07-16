#pragma once

#include <string>
#include <vector>

#include "palette.hpp"

namespace spratgen {

class FrameExporter {
public:
    void writeFrame(const Image& image, const std::string& path) const;
    void finalizeMetadata(const std::vector<std::string>& framePaths, const std::string& outputPath) const;
};

}  // namespace spratgen