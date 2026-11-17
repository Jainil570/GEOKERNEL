#include "geokernel/algorithms/MollerTrumboreIntersector.hpp"

#include <cmath>

#include "geokernel/math/Tolerance.hpp"

namespace gk::algorithms {

std::optional<TriangleHit> MollerTrumboreIntersector::intersect(const geometry::Ray& ray,
                                                                const geometry::Triangle& tri,
                                                                CullMode cull) const noexcept {
    using math::Vec3;

    const Vec3 e1 = tri.b - tri.a;
    const Vec3 e2 = tri.c - tri.a;
    const Vec3 p = ray.direction.cross(e2);
    const double det = e1.dot(p);  // == -dot(direction, normal)

    if (cull == CullMode::BackFaces) {
        if (det < math::kEpsilon) {  // back-facing or parallel
            return std::nullopt;
        }
    } else if (std::abs(det) < math::kEpsilon) {  // parallel or degenerate
        return std::nullopt;
    }

    const double invDet = 1.0 / det;
    const Vec3 s = ray.origin - tri.a;
    const double u = s.dot(p) * invDet;
    if (u < 0.0 || u > 1.0) {
        return std::nullopt;
    }

    const Vec3 q = s.cross(e1);
    const double v = ray.direction.dot(q) * invDet;
    if (v < 0.0 || u + v > 1.0) {
        return std::nullopt;
    }

    const double t = e2.dot(q) * invDet;
    if (!ray.inRange(t)) {
        return std::nullopt;
    }
    return TriangleHit{t, u, v, det > 0.0};
}

}  // namespace gk::algorithms
