#include "generator.hpp"
#include "pose_model.hpp"

#include "libresprite_core/pixel_engine.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string input;
    std::string output;
    std::string animation;
    int frames = 0;
    std::string palettePath;
    std::string cleanup;
};

void print_usage() {
    std::cerr << "Usage: spratforge --input <png> --output <directory> --anim <name> --frames <count>"
              << " [--palette <file.gpl>] [--cleanup edge,whitespace,stray]\n";
}

bool require_value(int argc, char** argv, int& index, std::string& value, const char* flag) {
    if (index + 1 >= argc || argv[index + 1] == nullptr) {
        std::cerr << "spratforge: " << flag << " expects a value\n";
        return false;
    }
    value = argv[++index];
    return true;
}

bool parse_options(int argc, char** argv, Options& options) {
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index] != nullptr ? argv[index] : "";
        if (argument == "--input") {
            if (!require_value(argc, argv, index, options.input, "--input")) return false;
        } else if (argument == "--output") {
            if (!require_value(argc, argv, index, options.output, "--output")) return false;
        } else if (argument == "--anim") {
            if (!require_value(argc, argv, index, options.animation, "--anim")) return false;
        } else if (argument == "--frames") {
            std::string rawFrames;
            if (!require_value(argc, argv, index, rawFrames, "--frames")) return false;
            try {
                options.frames = std::stoi(rawFrames);
            } catch (...) {
                std::cerr << "spratforge: --frames expects an integer\n";
                return false;
            }
        } else if (argument == "--palette") {
            if (!require_value(argc, argv, index, options.palettePath, "--palette")) return false;
        } else if (argument == "--cleanup") {
            if (!require_value(argc, argv, index, options.cleanup, "--cleanup")) return false;
        } else if (argument == "--help" || argument == "-h") {
            print_usage();
            std::exit(EXIT_SUCCESS);
        } else {
            std::cerr << "spratforge: unknown argument: " << argument << '\n';
            return false;
        }
    }

    if (options.input.empty() || options.output.empty() || options.animation.empty() || options.frames <= 0) {
        print_usage();
        return false;
    }
    const spratgen::PoseModel poseModel;
    const std::vector<std::string> names = poseModel.getAnimationNames();
    if (std::find(names.begin(), names.end(), options.animation) == names.end()) {
        std::cerr << "spratforge: invalid animation template: " << options.animation << '\n';
        return false;
    }
    return true;
}

spratgen::Image to_spratgen_image(const libresprite_core::Image& image) {
    spratgen::Image result;
    result.width = image.width;
    result.height = image.height;
    result.pixels.reserve(image.pixels.size());
    for (const libresprite_core::Rgba& pixel : image.pixels) {
        result.pixels.push_back(spratgen::Color{pixel.red, pixel.green, pixel.blue, pixel.alpha});
    }
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parse_options(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    libresprite_core::Image source;
    bool hasAlpha = false;
    std::string error;
    if (!libresprite_core::load_png(options.input, source, hasAlpha, error)) {
        std::cerr << "spratforge: " << error << '\n';
        return EXIT_FAILURE;
    }
    if (source.width != 400 || source.height != 400) {
        std::cerr << "spratforge: input canvas must be 400x400\n";
        return EXIT_FAILURE;
    }
    if (!hasAlpha) {
        std::cerr << "spratforge: input PNG must include an alpha channel\n";
        return EXIT_FAILURE;
    }

    std::vector<libresprite_core::CleanupPass> cleanupPasses;
    if (!libresprite_core::parse_cleanup_passes(options.cleanup, cleanupPasses, error)) {
        std::cerr << "spratforge: " << error << '\n';
        return EXIT_FAILURE;
    }
    libresprite_core::apply_cleanup(source, cleanupPasses);

    if (!options.palettePath.empty()) {
        libresprite_core::Palette palette;
        if (!libresprite_core::load_gpl_palette(options.palettePath, palette, error)) {
            std::cerr << "spratforge: " << error << '\n';
            return EXIT_FAILURE;
        }
        libresprite_core::enforce_palette(source, palette);
    }

    spratgen::Config config;
    config.outputDir = options.output;
    config.animName = options.animation;
    config.frameCount = options.frames;
    spratgen::Generator generator(config);
    generator.setAnimation(options.animation, options.frames);
    const std::vector<spratgen::RenderedFrame> frames = generator.generateFrames(to_spratgen_image(source));
    if (static_cast<int>(frames.size()) != options.frames) {
        std::cerr << "spratforge: animation generation failed\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}