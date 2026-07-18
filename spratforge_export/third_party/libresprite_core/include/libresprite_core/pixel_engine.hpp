#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace libresprite_core {

struct Rgba {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 0;
};

struct Image {
    int width = 0;
    int height = 0;
    std::vector<Rgba> pixels;
};

struct Palette {
    std::vector<Rgba> colors;
};

enum class CleanupPass {
    Edge,
    Stray,
    Whitespace,
};

bool load_png(const std::string& path, Image& image, bool& hasAlpha, std::string& error);
bool write_png(const Image& image, const std::string& path, std::string& error);
bool load_gpl_palette(const std::string& path, Palette& palette, std::string& error);
bool parse_cleanup_passes(const std::string& value, std::vector<CleanupPass>& passes, std::string& error);
void enforce_palette(Image& image, const Palette& palette);
void apply_cleanup(Image& image, const std::vector<CleanupPass>& passes);

}  // namespace libresprite_core