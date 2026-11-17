#pragma once
/// Shared fixtures and builders for GeoKernel tests.

#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "geokernel/geometry/Mesh.hpp"
#include "geokernel/geometry/Triangle.hpp"

namespace gk::test {

using geometry::Mesh;
using geometry::Triangle;
using math::Vec3;

/// Axis-aligned unit cube [0,1]^3: 8 vertices, 12 outward-facing CCW triangles.
inline Mesh makeUnitCube() {
    std::vector<Vec3> v;
    for (int i = 0; i < 8; ++i) {  // index = x + 2y + 4z
        v.emplace_back(i & 1 ? 1.0 : 0.0, i & 2 ? 1.0 : 0.0, i & 4 ? 1.0 : 0.0);
    }
    std::vector<Mesh::Face> f{
        {0, 2, 3}, {0, 3, 1},  // z = 0  (-Z)
        {4, 5, 7}, {4, 7, 6},  // z = 1  (+Z)
        {0, 1, 5}, {0, 5, 4},  // y = 0  (-Y)
        {2, 6, 7}, {2, 7, 3},  // y = 1  (+Y)
        {0, 4, 6}, {0, 6, 2},  // x = 0  (-X)
        {1, 3, 7}, {1, 7, 5},  // x = 1  (+X)
    };
    return Mesh(std::move(v), std::move(f), "unit_cube");
}

/// Right triangle in the z = 0 plane with +Z normal.
inline Triangle makeXYTriangle() {
    return Triangle{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
}

inline void appendU32LE(std::string& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(static_cast<char>((value >> (8 * i)) & 0xFFU));
    }
}

inline void appendF32LE(std::string& out, float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof bits);
    appendU32LE(out, bits);
}

/// Serialises triangles to a binary STL byte string.
/// `declaredCount` lets tests forge a corrupt header (defaults to the real count).
inline std::string makeBinaryStl(const std::vector<Triangle>& tris, std::string header = "binary test",
                                 long long declaredCount = -1) {
    header.resize(80, '\0');
    std::string out = header;
    appendU32LE(out, static_cast<std::uint32_t>(declaredCount < 0 ? static_cast<long long>(tris.size())
                                                                  : declaredCount));
    for (const auto& t : tris) {
        const Vec3 n = t.normal();
        for (const Vec3& p : {n, t.a, t.b, t.c}) {
            appendF32LE(out, static_cast<float>(p.x));
            appendF32LE(out, static_cast<float>(p.y));
            appendF32LE(out, static_cast<float>(p.z));
        }
        out.push_back('\0');  // 2-byte attribute
        out.push_back('\0');
    }
    return out;
}

/// All 12 triangles of a mesh, in face order.
inline std::vector<Triangle> trianglesOf(const Mesh& mesh) {
    std::vector<Triangle> out;
    for (std::size_t i = 0; i < mesh.faceCount(); ++i) {
        out.push_back(mesh.triangle(i));
    }
    return out;
}

/// RAII temporary file, removed on destruction.
class TempFile {
public:
    TempFile(std::string_view extension, const std::string& content) {
        static std::atomic<int> counter{0};
        path_ = std::filesystem::temp_directory_path() /
                ("geokernel_test_" + std::to_string(counter++) + std::string(extension));
        std::ofstream(path_, std::ios::binary) << content;
    }
    ~TempFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    TempFile(TempFile&&) = delete;
    TempFile& operator=(TempFile&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

/// Absolute path of a file under tests/data (directory injected by CMake).
inline std::filesystem::path dataFile(std::string_view name) {
    return std::filesystem::path(GK_TEST_DATA_DIR) / std::string(name);
}

}  // namespace gk::test
