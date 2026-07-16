#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "palette.hpp"
#include "pose_interp.hpp"
#include "silhouette.hpp"

namespace spratgen {

struct Palette {
    Color base;
    Color accent;
};

struct RenderedFrame {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba;
};

class PixelRenderer {
public:
    RenderedFrame renderFrame(const Silhouette& silhouette, const PoseSkeleton& posed, const Palette& palette);

private:
    void clear(RenderedFrame& out, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
    void drawOutline(RenderedFrame& out, const Silhouette& silhouette);
    void drawBody(RenderedFrame& out, const Silhouette& silhouette, const Palette& palette);
    void applyPose(RenderedFrame& out, const PoseSkeleton& posed);
    void putPixel(RenderedFrame& out, int x, int y, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
};

}  // namespace spratgen