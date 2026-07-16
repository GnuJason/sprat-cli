#include "export.hpp"

#include "pose_model.hpp"

#include <stb_image_write.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace spratgen {

bool FrameExporter::writePNG(const RenderedFrame& frame, const std::string& path) {
    if (frame.width <= 0 || frame.height <= 0 || frame.rgba.empty()) {
        return false;
    }

    return stbi_write_png(
        path.c_str(),
        frame.width,
        frame.height,
        4,
        frame.rgba.data(),
        frame.width * 4) != 0;
}

bool FrameExporter::writeFrame(const RenderedFrame& frame, const std::string& path) {
    return writePNG(frame, path);
}

bool FrameExporter::finalizeMetadata(
    const std::string& directory,
    const std::string& animationName,
    bool loop,
    int frameCount,
    int width,
    int height) {
    std::ofstream output(directory + "/metadata.json");
    if (!output) {
        return false;
    }

    output << "{\n";
    output << "  \"name\": \"spratgen_export\",\n";
    output << "  \"animation\": \"" << animationName << "\",\n";
    output << "  \"loop\": " << (loop ? "true" : "false") << ",\n";
    output << "  \"frameCount\": " << frameCount << ",\n";
    output << "  \"width\": " << width << ",\n";
    output << "  \"height\": " << height << ",\n";
    output << "  \"keyframes\": [";
    const PoseModel poseModel;
    const auto animationNames = poseModel.getAnimationNames();
    if (std::find(animationNames.begin(), animationNames.end(), animationName) != animationNames.end()) {
        const auto& animationTemplate = poseModel.getTemplate(animationName);
        for (std::size_t keyframeIndex = 0; keyframeIndex < animationTemplate.keyframes.size(); ++keyframeIndex) {
            if (keyframeIndex > 0) {
                output << ", ";
            }

            const auto& keyframe = animationTemplate.keyframes[keyframeIndex];
            output << "{"
                   << "\"t\": " << keyframe.t << ", "
                   << "\"curve\": \"" << keyframe.curve << "\""
                   << "}";
        }
    }
    output << "],\n";
    output << "  \"framePaths\": [";
    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        if (frameIndex > 0) {
            output << ", ";
        }
        std::ostringstream name;
        name << animationName << "_frame_" << std::setw(3) << std::setfill('0') << frameIndex << ".png";
        output << '"' << name.str() << '"';
    }
    output << "]\n";
    output << "}\n";
    return true;
}

}  // namespace spratgen