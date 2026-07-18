#include "libresprite_core/pixel_engine.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <queue>
#include <sstream>
#include <string_view>

namespace libresprite_core {

namespace {

std::size_t pixel_index(int x, int y, int width) {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x);
}

std::string trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    return first >= last ? std::string{} : std::string(first, last);
}

int color_distance_squared(const Rgba& left, const Rgba& right) {
    const int red = static_cast<int>(left.red) - static_cast<int>(right.red);
    const int green = static_cast<int>(left.green) - static_cast<int>(right.green);
    const int blue = static_cast<int>(left.blue) - static_cast<int>(right.blue);
    return red * red + green * green + blue * blue;
}

void remove_stray_components(Image& image) {
    if (image.width <= 0 || image.height <= 0 || image.pixels.empty()) {
        return;
    }

    std::vector<std::uint8_t> visited(image.pixels.size(), 0);
    constexpr std::array<std::pair<int, int>, 8> neighbors = {{{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            const std::size_t start = pixel_index(x, y, image.width);
            if (visited[start] != 0 || image.pixels[start].alpha < 32) {
                continue;
            }

            std::vector<std::size_t> component;
            std::queue<std::pair<int, int>> queue;
            queue.emplace(x, y);
            visited[start] = 1;
            while (!queue.empty()) {
                const auto [currentX, currentY] = queue.front();
                queue.pop();
                component.push_back(pixel_index(currentX, currentY, image.width));
                for (const auto [deltaX, deltaY] : neighbors) {
                    const int nextX = currentX + deltaX;
                    const int nextY = currentY + deltaY;
                    if (nextX < 0 || nextY < 0 || nextX >= image.width || nextY >= image.height) {
                        continue;
                    }
                    const std::size_t next = pixel_index(nextX, nextY, image.width);
                    if (visited[next] != 0 || image.pixels[next].alpha < 32) {
                        continue;
                    }
                    visited[next] = 1;
                    queue.emplace(nextX, nextY);
                }
            }

            if (component.size() <= 2U) {
                for (const std::size_t index : component) {
                    image.pixels[index] = Rgba{};
                }
            }
        }
    }
}

void clear_transparent_pixels(Image& image) {
    for (Rgba& pixel : image.pixels) {
        if (pixel.alpha < 32) {
            pixel = Rgba{};
        }
    }
}

}  // namespace

bool load_png(const std::string& path, Image& image, bool& hasAlpha, std::string& error) {
    int width = 0;
    int height = 0;
    int channels = 0;
    if (stbi_info(path.c_str(), &width, &height, &channels) == 0) {
        error = "unable to read PNG header: " + path;
        return false;
    }

    unsigned char* rawPixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (rawPixels == nullptr) {
        error = "unable to decode PNG: " + path;
        return false;
    }

    image.width = width;
    image.height = height;
    image.pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (std::size_t index = 0; index < image.pixels.size(); ++index) {
        const std::size_t offset = index * 4U;
        image.pixels[index] = Rgba{rawPixels[offset], rawPixels[offset + 1U], rawPixels[offset + 2U], rawPixels[offset + 3U]};
    }
    stbi_image_free(rawPixels);
    hasAlpha = channels == 2 || channels == 4;
    return true;
}

bool write_png(const Image& image, const std::string& path, std::string& error) {
    if (image.width <= 0 || image.height <= 0 || image.pixels.size() != static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height)) {
        error = "invalid RGBA image buffer";
        return false;
    }

    const int result = stbi_write_png(path.c_str(), image.width, image.height, 4, image.pixels.data(), image.width * static_cast<int>(sizeof(Rgba)));
    if (result == 0) {
        error = "unable to write PNG: " + path;
        return false;
    }
    return true;
}

bool load_gpl_palette(const std::string& path, Palette& palette, std::string& error) {
    std::ifstream input(path);
    if (!input) {
        error = "unable to open palette: " + path;
        return false;
    }

    palette.colors.clear();
    std::string line;
    bool headerSeen = false;
    while (std::getline(input, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            continue;
        }
        if (!headerSeen) {
            if (trimmed != "GIMP Palette") {
                error = "not a GPL palette: " + path;
                return false;
            }
            headerSeen = true;
            continue;
        }
        if (trimmed.rfind("Name:", 0) == 0 || trimmed.rfind("Columns:", 0) == 0) {
            continue;
        }

        std::istringstream row(trimmed);
        int red = 0;
        int green = 0;
        int blue = 0;
        if (row >> red >> green >> blue && red >= 0 && red <= 255 && green >= 0 && green <= 255 && blue >= 0 && blue <= 255) {
            palette.colors.push_back(Rgba{static_cast<std::uint8_t>(red), static_cast<std::uint8_t>(green), static_cast<std::uint8_t>(blue), 255});
        }
    }

    if (!headerSeen || palette.colors.empty()) {
        error = "palette contains no colors: " + path;
        return false;
    }
    return true;
}

bool parse_cleanup_passes(const std::string& value, std::vector<CleanupPass>& passes, std::string& error) {
    passes.clear();
    std::istringstream input(value);
    std::string token;
    while (std::getline(input, token, ',')) {
        token = trim(token);
        if (token == "edge") {
            passes.push_back(CleanupPass::Edge);
        } else if (token == "stray") {
            passes.push_back(CleanupPass::Stray);
        } else if (token == "whitespace") {
            passes.push_back(CleanupPass::Whitespace);
        } else if (!token.empty()) {
            error = "unknown cleanup pass: " + token;
            return false;
        }
    }
    return true;
}

void enforce_palette(Image& image, const Palette& palette) {
    if (palette.colors.empty()) {
        return;
    }
    for (Rgba& pixel : image.pixels) {
        if (pixel.alpha == 0) {
            continue;
        }
        const auto nearest = std::min_element(palette.colors.begin(), palette.colors.end(), [&pixel](const Rgba& left, const Rgba& right) {
            return color_distance_squared(pixel, left) < color_distance_squared(pixel, right);
        });
        pixel.red = nearest->red;
        pixel.green = nearest->green;
        pixel.blue = nearest->blue;
    }
}

void apply_cleanup(Image& image, const std::vector<CleanupPass>& passes) {
    for (const CleanupPass pass : passes) {
        switch (pass) {
        case CleanupPass::Edge:
        case CleanupPass::Whitespace:
            clear_transparent_pixels(image);
            break;
        case CleanupPass::Stray:
            remove_stray_components(image);
            break;
        }
    }
}

}  // namespace libresprite_core