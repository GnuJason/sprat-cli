#pragma once

#include <cstdint>
#include <cstring>

namespace sprat::core {

// FNV-1a 64-bit hash function
// Reference: http://www.isthe.com/chongo/tech/comp/fnv/
inline uint64_t fnv1a_hash(const unsigned char* data, size_t len) {
    constexpr uint64_t FNV_64_PRIME = 0x100000001B3ULL;
    constexpr uint64_t FNV_64_OFFSET_BASIS = 0xCBF29CE484222325ULL;

    uint64_t hash = FNV_64_OFFSET_BASIS;
    for (size_t i = 0; i < len; ++i) {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= FNV_64_PRIME;
    }
    return hash;
}

} // namespace sprat::core
