#pragma once
/// @file Triangle.hpp
/// @brief Triangle by value with counter-clockwise winding (a -> b -> c)
/// defining the front face via the right-hand rule.

#include "geokernel/math/Tolerance.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::geometry {

using math::Vec3;

struct Triangle {
    Vec3 a{};
    Vec3 b{};
    Vec3 c{};

    /// (b - a) x (c - a); length == 2 * area.
    [[nodiscard]] constexpr Vec3 unnormalizedNormal() const noexcept { return (b - a).cross(c - a); }

    /// Unit normal, or the zero vector for a degenerate triangle.
    [[nodiscard]] Vec3 normal() const noexcept { return unnormalizedNormal().normalized(); }

    [[nodiscard]] double area() const noexcept { return 0.5 * unnormalizedNormal().length(); }

    [[nodiscard]] constexpr Vec3 centroid() const noexcept { return (a + b + c) / 3.0; }

    /// Point from barycentric coordinates: (1 - u - v) a + u b + v c.
    [[nodiscard]] constexpr Vec3 pointAt(double u, double v) const noexcept {
        return a * (1.0 - u - v) + b * u + c * v;
    }

    /// Zero (or near-zero) area: collinear or coincident vertices.
    [[nodiscard]] bool isDegenerate(double areaTolerance = math::kEpsilon) const noexcept {
        return area() <= areaTolerance;
    }
};

}  // namespace gk::geometry
