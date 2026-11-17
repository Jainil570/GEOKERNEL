#pragma once
/// @file Mesh.hpp
/// @brief Indexed triangle mesh (shared vertex list + index triples).
///
/// Invariants maintained by every mutating member:
///  * bounds() always equals the AABB of vertices()  (kept eagerly, no lazy
///    `mutable` cache, so const access is safe from multiple threads);
///  * transform() preserves outward winding even for mirroring transforms.
/// Index validity is guaranteed by addFace() and checked by validate() for
/// meshes built in bulk from external data.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "geokernel/Export.hpp"
#include "geokernel/core/Result.hpp"
#include "geokernel/geometry/AABB.hpp"
#include "geokernel/geometry/Triangle.hpp"
#include "geokernel/math/Transform.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::geometry {

class GK_EXPORT Mesh {
public:
    using Index = std::uint32_t;
    using Face = std::array<Index, 3>;

    Mesh() = default;
    /// Bulk construction (e.g. from a reader). Call validate() for untrusted data.
    Mesh(std::vector<Vec3> vertices, std::vector<Face> faces, std::string name = {});

    // ---- construction --------------------------------------------------
    Index addVertex(const Vec3& p);
    /// @throws std::out_of_range if any index does not reference an existing vertex.
    void addFace(Index a, Index b, Index c);
    void reserve(std::size_t vertexCount, std::size_t faceCount);
    void clear() noexcept;

    // ---- access ---------------------------------------------------------
    [[nodiscard]] const std::vector<Vec3>& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const std::vector<Face>& faces() const noexcept { return faces_; }
    [[nodiscard]] std::size_t vertexCount() const noexcept { return vertices_.size(); }
    [[nodiscard]] std::size_t faceCount() const noexcept { return faces_.size(); }
    [[nodiscard]] bool empty() const noexcept { return faces_.empty(); }

    /// @throws std::out_of_range for an invalid face index.
    [[nodiscard]] Triangle triangle(std::size_t faceIndex) const;

    [[nodiscard]] const AABB& bounds() const noexcept { return bounds_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    // ---- queries --------------------------------------------------------
    [[nodiscard]] double surfaceArea() const noexcept;
    [[nodiscard]] std::size_t countDegenerateFaces(double areaTolerance = math::kEpsilon) const noexcept;
    [[nodiscard]] std::vector<Vec3> computeFaceNormals() const;
    /// Area-weighted average of incident face normals, unit length.
    [[nodiscard]] std::vector<Vec3> computeVertexNormals() const;
    /// Approximate heap usage in bytes (vertex + index buffers).
    [[nodiscard]] std::size_t memoryFootprint() const noexcept;

    /// Checks index ranges and that all coordinates are finite.
    [[nodiscard]] core::Status validate() const;

    // ---- transforms -----------------------------------------------------
    void transform(const math::Transform& xf);
    [[nodiscard]] Mesh transformed(const math::Transform& xf) const;

private:
    void recomputeBounds() noexcept;

    std::vector<Vec3> vertices_;
    std::vector<Face> faces_;
    AABB bounds_;
    std::string name_;
};

}  // namespace gk::geometry
