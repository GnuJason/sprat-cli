#include "silhouette.hpp"

namespace spratgen {

Silhouette SilhouetteExtractor::extract(const Image& image) const {
    Silhouette silhouette;
    silhouette.width = image.width;
    silhouette.height = image.height;
    silhouette.mask.assign(static_cast<std::size_t>(image.width > 0 && image.height > 0 ? image.width * image.height : 0), 1);
    return silhouette;
}

}  // namespace spratgen