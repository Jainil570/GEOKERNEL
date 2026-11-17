#pragma once
/// @file Ray.hpp
/// @brief Parametric ray  P(t) = origin + t * direction,  t in [tMin, tMax].
///
/// The direction is deliberately NOT normalised on construction: keeping it
/// as given means a ray transformed into another space keeps the same
/// t-values, so hits found in local space map 1:1 back to world space.

#include <limits>

#include "geokernel/math/Transform.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::geometry {

using math::Vec3;

struct Ray {
    Vec3 origin{};
    Vec3 direction{0.0, 0.0, 1.0};
    double tMin = 0.0;
    double tMax = std::numeric_limits<double>::infinity();

    Ray() noexcept = default;
    Ray(const Vec3& o, const Vec3& d, double minT = 0.0,
        double maxT = std::numeric_limits<double>::infinity()) noexcept
        : origin(o), direction(d), tMin(minT), tMax(maxT) {}

    /// Segment-style ray from `from` towards `to`: unit direction, tMax = |to - from|.
    [[nodiscard]] static Ray fromPoints(const Vec3& from, const Vec3& to) noexcept {
        const Vec3 d = to - from;
        return Ray{from, d.normalized(), 0.0, d.length()};
    }

    [[nodiscard]] Vec3 at(double t) const noexcept { return origin + direction * t; }

    /// Finite origin/direction, non-zero direction and tMin <= tMax.
    [[nodiscard]] bool isValid() const noexcept {
        return origin.isFinite() && direction.isFinite() && !direction.isZero() && tMin <= tMax;
    }

    [[nodiscard]] bool inRange(double t) const noexcept { return t >= tMin && t <= tMax; }

    /// Same parametrisation in the target space (direction is not renormalised).
    [[nodiscard]] Ray transformed(const math::Transform& xf) const noexcept {
        return Ray{xf.applyToPoint(origin), xf.applyToVector(direction), tMin, tMax};
    }
};

}  // namespace gk::geometry
