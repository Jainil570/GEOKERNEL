#pragma once
/// @file VertexWelder.hpp
/// @brief PRIVATE: de-duplicates bit-identical vertices while building a mesh.
/// STL repeats each shared vertex once per incident facet (~6x for closed
/// meshes); welding restores connectivity and cuts memory roughly in half.

#include <array>
#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "geokernel/geometry/Mesh.hpp"

namespace gk::io::detail {

class VertexWelder {
public:
    VertexWelder(geometry::Mesh& mesh, bool enabled) : mesh_(mesh), enabled_(enabled) {}

    geometry::Mesh::Index add(const math::Vec3& p) {
        if (!enabled_) {
            return mesh_.addVertex(p);
        }
        const Key key{bits(p.x), bits(p.y), bits(p.z)};
        const auto it = lookup_.find(key);
        if (it != lookup_.end()) {
            return it->second;
        }
        const auto index = mesh_.addVertex(p);
        lookup_.emplace(key, index);
        return index;
    }

    void reserve(std::size_t n) { lookup_.reserve(n); }

private:
    using Key = std::array<std::uint64_t, 3>;

    struct KeyHash {
        std::size_t operator()(const Key& k) const noexcept {
            std::uint64_t h = 1469598103934665603ULL;
            for (const auto v : k) {
                h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6U) + (h >> 2U);
            }
            return static_cast<std::size_t>(h);
        }
    };

    static std::uint64_t bits(double v) noexcept {
        if (v == 0.0) {
            v = 0.0;  // fold -0.0 into +0.0 so they weld together
        }
        std::uint64_t out = 0;
        std::memcpy(&out, &v, sizeof out);
        return out;
    }

    geometry::Mesh& mesh_;
    bool enabled_;
    std::unordered_map<Key, geometry::Mesh::Index, KeyHash> lookup_;
};

}  // namespace gk::io::detail
