#include "export.hpp"

#include <fstream>

namespace spratgen {

void FrameExporter::writeFrame(const Image& image, const std::string& path) const {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return;
    }

    output << "P3\n" << image.width << ' ' << image.height << "\n255\n";
    const Color color = image.pixels.empty() ? Color{} : image.pixels.front();
    const int pixelCount = image.width > 0 && image.height > 0 ? image.width * image.height : 1;
    for (int index = 0; index < pixelCount; ++index) {
        output << static_cast<int>(color.red) << ' '
               << static_cast<int>(color.green) << ' '
               << static_cast<int>(color.blue) << '\n';
    }
}

void FrameExporter::finalizeMetadata(const std::vector<std::string>& framePaths, const std::string& outputPath) const {
    std::ofstream output(outputPath);
    if (!output) {
        return;
    }

    for (const std::string& framePath : framePaths) {
        output << framePath << '\n';
    }
}

}  // namespace spratgen