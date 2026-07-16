#include "generator.hpp"

#include "pose_model.hpp"

#include <stb_image.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <utility>

namespace spratgen {

namespace {

bool has_named_pose(const Skeleton& skeleton) {
    return skeleton.head.name == "head"
        || skeleton.torso.name == "torso"
        || skeleton.left_arm.name == "left_arm"
        || skeleton.right_arm.name == "right_arm"
        || skeleton.left_leg.name == "left_leg"
        || skeleton.right_leg.name == "right_leg";
}

Skeleton make_render_skeleton(const PoseSkeleton& posed) {
    Skeleton skeleton;
    skeleton.head = Joint("head", posed.head.x, posed.head.y);
    skeleton.torso = Joint("torso", posed.torso.x, posed.torso.y);
    skeleton.left_arm = Joint("left_arm", posed.left_arm.x, posed.left_arm.y);
    skeleton.right_arm = Joint("right_arm", posed.right_arm.x, posed.right_arm.y);
    skeleton.left_leg = Joint("left_leg", posed.left_leg.x, posed.left_leg.y);
    skeleton.right_leg = Joint("right_leg", posed.right_leg.x, posed.right_leg.y);
    skeleton.joints = {
        skeleton.head,
        skeleton.torso,
        skeleton.left_arm,
        skeleton.right_arm,
        skeleton.left_leg,
        skeleton.right_leg,
    };
    return skeleton;
}

Skeleton apply_pose_offset(const Skeleton& skeleton, const PoseSkeleton& offset) {
    Skeleton target = skeleton;
    target.head.x += offset.head.x;
    target.head.y += offset.head.y;
    target.torso.x += offset.torso.x;
    target.torso.y += offset.torso.y;
    target.left_arm.x += offset.left_arm.x;
    target.left_arm.y += offset.left_arm.y;
    target.right_arm.x += offset.right_arm.x;
    target.right_arm.y += offset.right_arm.y;
    target.left_leg.x += offset.left_leg.x;
    target.left_leg.y += offset.left_leg.y;
    target.right_leg.x += offset.right_leg.x;
    target.right_leg.y += offset.right_leg.y;
    target.joints = {
        target.head,
        target.torso,
        target.left_arm,
        target.right_arm,
        target.left_leg,
        target.right_leg,
    };
    return target;
}

float ease_in(float t) {
    return t * t;
}

float ease_out(float t) {
    const float inverse = 1.0f - t;
    return 1.0f - inverse * inverse;
}

float ease_in_out(float t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    }

    const float inverse = -2.0f * t + 2.0f;
    return 1.0f - (inverse * inverse) / 2.0f;
}

float ease_cubic(float t) {
    return t * t * t;
}

float apply_animation_easing(const std::string& animationName, float t) {
    if (animationName == "idle") {
        return ease_in_out(t);
    }
    if (animationName == "jab" || animationName == "uppercut") {
        return ease_out(t);
    }
    if (animationName == "hook") {
        return ease_cubic(t);
    }
    if (animationName == "hit" || animationName == "knockdown") {
        return ease_in(t);
    }
    return t;
}

struct KeyframeSpan {
    Keyframe from;
    Keyframe to;
    float localT = 0.0f;
};

KeyframeSpan resolve_keyframe_span(const AnimationTemplate& animationTemplate, float t) {
    if (animationTemplate.keyframes.empty()) {
        return {};
    }

    const float clampedT = std::clamp(t, 0.0f, 1.0f);
    if (animationTemplate.keyframes.size() == 1) {
        return KeyframeSpan{animationTemplate.keyframes.front(), animationTemplate.keyframes.front(), 0.0f};
    }

    for (std::size_t index = 0; index + 1 < animationTemplate.keyframes.size(); ++index) {
        const Keyframe& current = animationTemplate.keyframes[index];
        const Keyframe& next = animationTemplate.keyframes[index + 1];
        if (clampedT <= next.t) {
            const float span = next.t - current.t;
            const float localT = span > 0.0f ? (clampedT - current.t) / span : 0.0f;
            return KeyframeSpan{current, next, std::clamp(localT, 0.0f, 1.0f)};
        }
    }

    return KeyframeSpan{animationTemplate.keyframes.back(), animationTemplate.keyframes.back(), 0.0f};
}

}  // namespace

Generator::Generator(Config config)
    : config_(std::move(config)) {}

Image Generator::loadMasterFrame() {
    Image image;
    if (config_.args.size() > 1) {
        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* rawPixels = stbi_load(config_.args[1].c_str(), &width, &height, &channels, 4);
        if (rawPixels != nullptr) {
            image.width = width;
            image.height = height;
            const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
            image.pixels.reserve(pixelCount);
            for (std::size_t index = 0; index < pixelCount; ++index) {
                const std::size_t base = index * 4U;
                image.pixels.push_back(Color{
                    rawPixels[base],
                    rawPixels[base + 1U],
                    rawPixels[base + 2U],
                    rawPixels[base + 3U],
                });
            }
            stbi_image_free(rawPixels);
        }
    }

    if (image.pixels.empty()) {
        image.width = 1;
        image.height = 1;
        image.pixels.push_back(Color{});
    }

    silhouette_ = silhouetteExtractor_.extract(image);
    return image;
}

Palette Generator::setupPalette(const Image& image) const {
    const std::vector<Color> colors = paletteManager_.quantize(paletteManager_.extractFromImage(image), 16);
    Palette palette;
    if (!colors.empty()) {
        palette.base = colors.front();
        palette.accent = colors.size() > 1 ? colors[1] : colors.front();
    }
    return palette;
}

Skeleton Generator::buildSkeleton(const Image& image) {
    if (silhouette_.mask.empty() || silhouette_.width != image.width || silhouette_.height != image.height) {
        silhouette_ = silhouetteExtractor_.extract(image);
    }
    skeleton_ = skeletonBuilder_.build(silhouette_);
    return skeleton_;
}

void Generator::setupPoseModel() {
    if (!has_named_pose(skeleton_)) {
        Image image;
        image.width = silhouette_.width;
        image.height = silhouette_.height;
        if (!silhouette_.mask.empty()) {
            buildSkeleton(image);
        }
    }

    poseModel_.loadDefaults();
}

std::vector<RenderedFrame> Generator::generateFrames(const std::string& animType, std::size_t /*frameCount*/) {
    const Image masterFrame = loadMasterFrame();
    const Palette palette = setupPalette(masterFrame);
    const Skeleton inferredSkeleton = buildSkeleton(masterFrame);
    setupPoseModel();
    const AnimationTemplate& animationTemplate = poseModel_.getTemplate(animType);
    const std::size_t totalFrames = static_cast<std::size_t>(poseModel_.getFrameCount(animType));

    std::vector<RenderedFrame> frames;
    frames.reserve(totalFrames);
    const std::string outputDir = ".";

    for (std::size_t frameIndex = 0; frameIndex < totalFrames; ++frameIndex) {
        const float t = totalFrames > 1
            ? static_cast<float>(frameIndex) / static_cast<float>(totalFrames - 1)
            : 0.0f;
        const KeyframeSpan span = resolve_keyframe_span(animationTemplate, t);
        const Skeleton baseSkeleton = apply_pose_offset(inferredSkeleton, span.from.pose);
        const Skeleton targetSkeleton = apply_pose_offset(inferredSkeleton, span.to.pose);
        const PoseSkeleton posed = poseInterp_.apply(
            baseSkeleton,
            targetSkeleton,
            apply_animation_easing(animationTemplate.name, span.localT));
        RenderedFrame frame = renderer_.renderFrame(silhouette_, posed, palette);
        std::ostringstream fileName;
        fileName << outputDir << "/frame_" << std::setw(3) << std::setfill('0') << frameIndex << ".png";
        static_cast<void>(exporter_.writeFrame(frame, fileName.str()));
        frames.push_back(std::move(frame));
    }

    static_cast<void>(exporter_.finalizeMetadata(
        outputDir,
        static_cast<int>(totalFrames),
        silhouette_.width,
        silhouette_.height));
    return frames;
}

const Silhouette& Generator::silhouette() const {
    return silhouette_;
}

const Skeleton& Generator::skeleton() const {
    return skeleton_;
}

}  // namespace spratgen