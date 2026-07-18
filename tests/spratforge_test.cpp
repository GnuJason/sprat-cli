#include "libresprite_core/pixel_engine.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool files_match(const std::filesystem::path& left, const std::filesystem::path& right) {
    std::ifstream leftInput(left, std::ios::binary);
    std::ifstream rightInput(right, std::ios::binary);
    return std::vector<char>(std::istreambuf_iterator<char>(leftInput), {})
        == std::vector<char>(std::istreambuf_iterator<char>(rightInput), {});
}

bool metadata_has_expected_dimensions(const std::filesystem::path& metadata) {
    std::ifstream input(metadata);
    const std::string contents(std::istreambuf_iterator<char>(input), {});
    return contents.find("\"frameCount\": 3") != std::string::npos
        && contents.find("\"width\": 400") != std::string::npos
        && contents.find("\"height\": 400") != std::string::npos;
}

int run_forge(const std::filesystem::path& executable, const std::filesystem::path& input, const std::filesystem::path& output, const std::filesystem::path& palette) {
    const std::string command = "\"" + executable.string() + "\" --input \"" + input.string()
        + "\" --output \"" + output.string() + "\" --anim jab --frames 3 --palette \""
        + palette.string() + "\" --cleanup edge,whitespace,stray";
    return std::system(command.c_str());
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "spratforge_test expects the spratforge executable path\n";
        return EXIT_FAILURE;
    }

    const std::filesystem::path workspace = std::filesystem::temp_directory_path() / "spratforge_test";
    std::error_code error;
    std::filesystem::remove_all(workspace, error);
    std::filesystem::create_directories(workspace, error);
    if (error) {
        std::cerr << "Unable to create test workspace: " << error.message() << '\n';
        return EXIT_FAILURE;
    }

    libresprite_core::Image image;
    image.width = 400;
    image.height = 400;
    image.pixels.assign(400U * 400U, libresprite_core::Rgba{});
    for (int y = 120; y < 280; ++y) {
        for (int x = 160; x < 240; ++x) {
            image.pixels[static_cast<std::size_t>(y) * 400U + static_cast<std::size_t>(x)] = libresprite_core::Rgba{210, 50, 50, 255};
        }
    }
    image.pixels[0] = libresprite_core::Rgba{1, 2, 3, 255};
    image.pixels[1] = libresprite_core::Rgba{1, 2, 3, 255};
    image.pixels[2] = libresprite_core::Rgba{90, 90, 90, 10};

    const std::filesystem::path input = workspace / "master.png";
    const std::filesystem::path palette = workspace / "palette.gpl";
    const std::filesystem::path firstOutput = workspace / "first";
    const std::filesystem::path secondOutput = workspace / "second";
    std::string writeError;
    if (!libresprite_core::write_png(image, input.string(), writeError)) {
        std::cerr << writeError << '\n';
        return EXIT_FAILURE;
    }
    {
        std::ofstream paletteOutput(palette);
        paletteOutput << "GIMP Palette\nName: test\nColumns: 2\n#\n0 0 0 Black\n255 0 0 Red\n";
    }

    if (run_forge(argv[1], input, firstOutput, palette) != 0
        || run_forge(argv[1], input, secondOutput, palette) != 0) {
        std::cerr << "spratforge command failed\n";
        return EXIT_FAILURE;
    }

    for (int frameIndex = 0; frameIndex < 3; ++frameIndex) {
        const std::string filename = "jab_frame_00" + std::to_string(frameIndex) + ".png";
        if (!std::filesystem::exists(firstOutput / filename) || !files_match(firstOutput / filename, secondOutput / filename)) {
            std::cerr << "Generated frame is missing or nondeterministic: " << filename << '\n';
            return EXIT_FAILURE;
        }
    }
    if (!metadata_has_expected_dimensions(firstOutput / "metadata.json")
        || !files_match(firstOutput / "metadata.json", secondOutput / "metadata.json")) {
        std::cerr << "Metadata is missing expected dimensions or is nondeterministic\n";
        return EXIT_FAILURE;
    }

    std::filesystem::remove_all(workspace, error);
    return EXIT_SUCCESS;
}