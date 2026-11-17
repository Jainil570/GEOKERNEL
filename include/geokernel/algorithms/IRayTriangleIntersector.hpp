#pragma once
/// @file IRayTriangleIntersector.hpp
/// @brief Interface for ray/triangle intersection kernels.
///
/// Several numerically different algorithms exist (Möller–Trumbore,
/// edge functions, watertight Woop et al.). Exposing them behind one
/// interface lets callers select an implementation by registry name and
/// lets tests cross-validate implementations against each other.

#include <optional>

#include "geokernel/core/IBase.hpp"
#include "geokernel/geometry/Ray.hpp"
#include "geokernel/geometry/Triangle.hpp"

namespace gk::algorithms {

enum class CullMode {
    None,       ///< report hits on both sides
    BackFaces,  ///< ignore triangles whose normal points away from the ray origin
};

struct TriangleHit {
    double t = 0.0;          ///< ray parameter of the hit
    double u = 0.0;          ///< barycentric weight of vertex b
    double v = 0.0;          ///< barycentric weight of vertex c
    bool frontFace = false;  ///< true when the ray travels against the triangle normal
};

class IRayTriangleIntersector : public virtual core::IBase {
    GK_DECLARE_INTERFACE("gk.algorithms.IRayTriangleIntersector")

public:
    /// @return the hit if the ray intersects the triangle with t in [tMin, tMax].
    /// Degenerate triangles and rays parallel to the triangle plane never hit.
    [[nodiscard]] virtual std::optional<TriangleHit> intersect(
        const geometry::Ray& ray, const geometry::Triangle& tri,
        CullMode cull = CullMode::None) const noexcept = 0;
};

}  // namespace gk::algorithms
