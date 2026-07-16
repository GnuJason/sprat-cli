#include "renderer.hpp"

#include <array>
#include <cstddef>

namespace spratgen {

namespace {

std::size_t pixel_offset(int x, int y, int width) {
    return (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4U;
}

}  // namespace

RenderedFrame PixelRenderer::renderFrame(const Silhouette& silhouette, const PoseSkeleton& posed, const Palette& palette) {
    RenderedFrame frame;
    frame.width = silhouette.width;
    frame.height = silhouette.height;
    const std::size_t pixelCount = frame.width > 0 && frame.height > 0
        ? static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height)
        : 0U;
    frame.rgba.resize(pixelCount * 4U, 0U);

    clear(frame, 32, 32, 32, 0);
    drawBody(frame, silhouette, palette);
    drawOutline(frame, silhouette);
    applyPose(frame, posed);
    return frame;
}

void PixelRenderer::clear(RenderedFrame& out, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    for (std::size_t offset = 0; offset + 3U < out.rgba.size(); offset += 4U) {
        out.rgba[offset] = red;
        out.rgba[offset + 1U] = green;
        out.rgba[offset + 2U] = blue;
        out.rgba[offset + 3U] = alpha;
    }
}

void PixelRenderer::drawOutline(RenderedFrame& out, const Silhouette& silhouette) {
    for (int y = 0; y < silhouette.height; ++y) {
        for (int x = 0; x < silhouette.width; ++x) {
            const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(silhouette.width) + static_cast<std::size_t>(x);
            if (index >= silhouette.outline.size() || silhouette.outline[index] == 0) {
                continue;
            }
            putPixel(out, x, y, 0, 0, 0, 255);
        }
    }
}

void PixelRenderer::drawBody(RenderedFrame& out, const Silhouette& silhouette, const Palette& palette) {
    const int torsoTop = silhouette.height / 3;
    const int torsoBottom = (silhouette.height * 2) / 3;
    for (int y = 0; y < silhouette.height; ++y) {
        for (int x = 0; x < silhouette.width; ++x) {
            const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(silhouette.width) + static_cast<std::size_t>(x);
            if (index >= silhouette.mask.size() || silhouette.mask[index] == 0) {
                continue;
            }

            const Color& color = (y >= torsoTop && y <= torsoBottom) ? palette.base : palette.accent;
            putPixel(out, x, y, color.red, color.green, color.blue, 255);
        }
    }
}

void PixelRenderer::applyPose(RenderedFrame& out, const PoseSkeleton& posed) {
    const std::array<PoseJoint, 6> joints = {
        posed.head,
        posed.torso,
        posed.left_arm,
        posed.right_arm,
        posed.left_leg,
        posed.right_leg,
    };

    for (const PoseJoint& joint : joints) {
        putPixel(out, joint.x, joint.y, 255, 255, 255, 255);
        putPixel(out, joint.x + 1, joint.y, 255, 255, 255, 255);
    }
}

void PixelRenderer::putPixel(RenderedFrame& out, int x, int y, std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    if (x < 0 || y < 0 || x >= out.width || y >= out.height) {
        return;
    }

    const std::size_t offset = pixel_offset(x, y, out.width);
    if (offset + 3U >= out.rgba.size()) {
        return;
    }

    out.rgba[offset] = red;
    out.rgba[offset + 1U] = green;
    out.rgba[offset + 2U] = blue;
    out.rgba[offset + 3U] = alpha;
}

}  // namespace spratgen