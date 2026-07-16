#pragma once

#include <string>

#include "renderer.hpp"

namespace spratgen {

class FrameExporter {
public:
    bool writeFrame(const RenderedFrame& frame, const std::string& path);
    bool finalizeMetadata(const std::string& directory, int frameCount, int width, int height);

private:
    bool writePNG(const RenderedFrame& frame, const std::string& path);
};

}  // namespace spratgen