#include "palette.hpp"

#include <algorithm>

namespace spratgen {

std::vector<Color> PaletteManager::extractFromImage(const Image& image) const {
    return image.pixels;
}

std::vector<Color> PaletteManager::quantize(const std::vector<Color>& colors, std::size_t maxColors) const {
    if (maxColors == 0 || colors.size() <= maxColors) {
        return colors;
    }

    return std::vector<Color>(colors.begin(), colors.begin() + static_cast<std::ptrdiff_t>(maxColors));
}

}  // namespace spratgen