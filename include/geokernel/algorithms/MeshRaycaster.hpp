#pragma once
/// @file MeshRaycaster.hpp
/// @brief Ray queries against a whole mesh, using any IRayTriangleIntersector.
///
/// Holds the mesh as shared_ptr<const Mesh>: the raycaster never mutates
/// geometry and may outlive the code that loaded it. All query methods are
/// const and safe to call concurrently from multiple threads.
///
/// Acceleration: the mesh AABB rejects missing rays early and clips the ray
/// interval. A BVH is the planned next step (see docs/ARCHITECTURE.md §8).

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "geokernel/Export.hpp"
#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/geometry/Mesh.hpp"
#include "geokernel/geometry/Ray.hpp"

namespace gk::algorithms {

struct MeshHit {
    double t = 0.0;
    std::size_t faceIndex = 0;
    double u = 0.0;
    double v = 0.0;
    math::Vec3 point{};
    math::Vec3 normal{};  ///< unit geometric normal of the hit face
    bool frontFace = false;
};

class GK_EXPORT MeshRaycaster {
public:
    /// Uses the default Möller–Trumbore kernel.
    explicit MeshRaycaster(std::shared_ptr<const geometry::Mesh> mesh);

    /// @throws std::invalid_argument if `mesh` or `intersector` is null.
    MeshRaycaster(std::shared_ptr<const geometry::Mesh> mesh,
                  std::shared_ptr<const IRayTriangleIntersector> intersector);

    /// Nearest hit along the ray, or nullopt.
    [[nodiscard]] std::optional<MeshHit> closestHit(const geometry::Ray& ray,
                                                    CullMode cull = CullMode::None) const;

    /// True as soon as any hit is found (shadow / occlusion queries).
    [[nodiscard]] bool anyHit(const geometry::Ray& ray, CullMode cull = CullMode::None) const;

    /// Every hit, sorted by increasing t.
    [[nodiscard]] std::vector<MeshHit> allHits(const geometry::Ray& ray,
                                               CullMode cull = CullMode::None) const;

    [[nodiscard]] const geometry::Mesh& mesh() const noexcept { return *mesh_; }
    [[nodiscard]] const IRayTriangleIntersector& intersector() const noexcept { return *intersector_; }

private:
    enum class Mode { Closest, Any, All };
    std::vector<MeshHit> trace(const geometry::Ray& ray, CullMode cull, Mode mode) const;

    std::shared_ptr<const geometry::Mesh> mesh_;
    std::shared_ptr<const IRayTriangleIntersector> intersector_;
};

}  // namespace gk::algorithms
