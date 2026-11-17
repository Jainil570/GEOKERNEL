#pragma once
/// @file AABB.hpp
/// @brief Axis-aligned bounding box. Default-constructed boxes are *empty*
/// (min = +inf, max = -inf) so that expand() works without special cases.

#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "geokernel/Export.hpp"
#include "geokernel/geometry/Ray.hpp"
#include "geokernel/math/Transform.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::geometry {

using math::Vec3;

class GK_EXPORT AABB {
public:
    AABB() noexcept;
    /// Box spanning two arbitrary corner points (order does not matter).
    AABB(const Vec3& p, const Vec3& q) noexcept;

    [[nodiscard]] static AABB fromPoints(const std::vector<Vec3>& points) noexcept;

    [[nodiscard]] bool isEmpty() const noexcept;

    void expand(const Vec3& p) noexcept;
    void expand(const AABB& other) noexcept;

    [[nodiscard]] const Vec3& min() const noexcept { return min_; }
    [[nodiscard]] const Vec3& max() const noexcept { return max_; }

    [[nodiscard]] Vec3 center() const noexcept;
    [[nodiscard]] Vec3 extent() const noexcept;  ///< max - min (zero for empty)
    [[nodiscard]] double surfaceArea() const noexcept;
    [[nodiscard]] double volume() const noexcept;
    [[nodiscard]] std::array<Vec3, 8> corners() const noexcept;

    [[nodiscard]] bool contains(const Vec3& p, double tolerance = math::kLinearTolerance) const noexcept;
    [[nodiscard]] bool intersects(const AABB& other) const noexcept;

    /// Slab test. Returns the [tEnter, tExit] interval clipped to the ray's
    /// [tMin, tMax], or nullopt if the ray misses. Handles axis-parallel rays.
    [[nodiscard]] std::optional<std::pair<double, double>> intersect(const Ray& ray) const noexcept;

    /// Conservative bound of the transformed box (transforms all 8 corners).
    [[nodiscard]] AABB transformed(const math::Transform& xf) const noexcept;

private:
    Vec3 min_;
    Vec3 max_;
};

}  // namespace gk::geometry
