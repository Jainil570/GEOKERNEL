#pragma once
/// @file Quat.hpp
/// @brief Unit quaternion for rotations (w + xi + yj + zk, Hamilton convention).

#include <iosfwd>

#include "geokernel/Export.hpp"
#include "geokernel/math/Mat4.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::math {

struct GK_EXPORT Quat {
    double w = 1.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    constexpr Quat() noexcept = default;
    constexpr Quat(double ww, double xx, double yy, double zz) noexcept : w(ww), x(xx), y(yy), z(zz) {}

    [[nodiscard]] static constexpr Quat identity() noexcept { return {}; }

    /// Rotation of `radians` about `axis` (normalised internally). Identity if axis ~ 0.
    [[nodiscard]] static Quat fromAxisAngle(const Vec3& axis, double radians) noexcept;

    /// Hamilton product: (a * b) applies b first, then a.
    [[nodiscard]] constexpr Quat operator*(const Quat& o) const noexcept {
        return {w * o.w - x * o.x - y * o.y - z * o.z, w * o.x + x * o.w + y * o.z - z * o.y,
                w * o.y - x * o.z + y * o.w + z * o.x, w * o.z + x * o.y - y * o.x + z * o.w};
    }

    [[nodiscard]] constexpr Quat conjugate() const noexcept { return {w, -x, -y, -z}; }
    [[nodiscard]] constexpr double normSquared() const noexcept { return w * w + x * x + y * y + z * z; }
    [[nodiscard]] double norm() const noexcept;
    [[nodiscard]] Quat normalized() const noexcept;
    /// Multiplicative inverse; identity for a ~zero quaternion.
    [[nodiscard]] Quat inverse() const noexcept;

    /// Rotates v by this (assumed unit) quaternion.
    [[nodiscard]] Vec3 rotate(const Vec3& v) const noexcept;

    [[nodiscard]] Mat4 toMatrix() const noexcept;

    /// Spherical linear interpolation along the shortest arc, t in [0, 1].
    [[nodiscard]] static Quat slerp(const Quat& a, const Quat& b, double t) noexcept;
};

/// Equality as rotations: q and -q represent the same rotation.
[[nodiscard]] GK_EXPORT bool almostEqualRotation(const Quat& a, const Quat& b,
                                                 double tolerance = kAngularTolerance) noexcept;

GK_EXPORT std::ostream& operator<<(std::ostream& os, const Quat& q);

}  // namespace gk::math
