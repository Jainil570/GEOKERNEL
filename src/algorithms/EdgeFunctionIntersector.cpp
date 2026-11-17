#include "geokernel/algorithms/EdgeFunctionIntersector.hpp"

#include <cmath>

#include "geokernel/math/Tolerance.hpp"

namespace gk::algorithms {

std::optional<TriangleHit> EdgeFunctionIntersector::intersect(const geometry::Ray& ray,
                                                              const geometry::Triangle& tri,
                                                              CullMode cull) const noexcept {
    using math::Vec3;

    const Vec3 n = tri.unnormalizedNormal();
    const double nn = n.dot(n);
    if (nn < math::kEpsilon * math::kEpsilon) {  // degenerate triangle
        return std::nullopt;
    }

    // 1) Ray / supporting plane.
    const double denom = n.dot(ray.direction);  // < 0 => ray hits the front face
    if (std::abs(denom) < math::kEpsilon) {
        return std::nullopt;
    }
    if (cull == CullMode::BackFaces && denom > 0.0) {
        return std::nullopt;
    }
    const double t = n.dot(tri.a - ray.origin) / denom;
    if (!ray.inRange(t)) {
        return std::nullopt;
    }

    // 2) Edge functions == signed sub-triangle areas (scaled by |n|^2).
    const Vec3 p = ray.at(t);
    const double wa = n.dot((tri.c - tri.b).cross(p - tri.b));
    const double wb = n.dot((tri.a - tri.c).cross(p - tri.c));
    const double wc = n.dot((tri.b - tri.a).cross(p - tri.a));
    if (wa < 0.0 || wb < 0.0 || wc < 0.0) {
        return std::nullopt;
    }
    return TriangleHit{t, wb / nn, wc / nn, denom < 0.0};
}

}  // namespace gk::algorithms
