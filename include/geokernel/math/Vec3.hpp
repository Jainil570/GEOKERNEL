#pragma once
/// @file Vec3.hpp
/// @brief 3D double-precision vector. Trivially copyable value type; all
/// operations are noexcept and (except length-based ones) constexpr.

#include <cmath>
#include <cstddef>
#include <iosfwd>

#include "geokernel/Export.hpp"
#include "geokernel/math/Tolerance.hpp"

namespace gk::math {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    constexpr Vec3() noexcept = default;
    constexpr Vec3(double xx, double yy, double zz) noexcept : x(xx), y(yy), z(zz) {}

    [[nodiscard]] static constexpr Vec3 zero() noexcept { return {0.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec3 unitX() noexcept { return {1.0, 0.0, 0.0}; }
    [[nodiscard]] static constexpr Vec3 unitY() noexcept { return {0.0, 1.0, 0.0}; }
    [[nodiscard]] static constexpr Vec3 unitZ() noexcept { return {0.0, 0.0, 1.0}; }

    /// Component access; index must be 0, 1 or 2 (anything else returns z).
    [[nodiscard]] constexpr double operator[](std::size_t i) const noexcept {
        return i == 0 ? x : (i == 1 ? y : z);
    }

    constexpr Vec3& operator+=(const Vec3& o) noexcept {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    constexpr Vec3& operator-=(const Vec3& o) noexcept {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }
    constexpr Vec3& operator*=(double s) noexcept {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr Vec3& operator/=(double s) noexcept {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    [[nodiscard]] constexpr double dot(const Vec3& o) const noexcept { return x * o.x + y * o.y + z * o.z; }

    [[nodiscard]] constexpr Vec3 cross(const Vec3& o) const noexcept {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }

    [[nodiscard]] constexpr double lengthSquared() const noexcept { return dot(*this); }
    [[nodiscard]] double length() const noexcept { return std::sqrt(lengthSquared()); }

    /// Unit vector in the same direction; the zero vector if length < kEpsilon.
    [[nodiscard]] Vec3 normalized() const noexcept {
        const double len = length();
        return len < kEpsilon ? zero() : Vec3{x / len, y / len, z / len};
    }

    [[nodiscard]] bool isFinite() const noexcept {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }

    [[nodiscard]] bool isZero(double tolerance = kEpsilon) const noexcept {
        return lengthSquared() <= tolerance * tolerance;
    }
};

[[nodiscard]] constexpr Vec3 operator+(Vec3 a, const Vec3& b) noexcept {
    return a += b;
}
[[nodiscard]] constexpr Vec3 operator-(Vec3 a, const Vec3& b) noexcept {
    return a -= b;
}
[[nodiscard]] constexpr Vec3 operator-(const Vec3& v) noexcept {
    return {-v.x, -v.y, -v.z};
}
[[nodiscard]] constexpr Vec3 operator*(Vec3 v, double s) noexcept {
    return v *= s;
}
[[nodiscard]] constexpr Vec3 operator*(double s, Vec3 v) noexcept {
    return v *= s;
}
[[nodiscard]] constexpr Vec3 operator/(Vec3 v, double s) noexcept {
    return v /= s;
}

/// Exact component-wise equality. Use almostEqual() for geometry.
[[nodiscard]] constexpr bool operator==(const Vec3& a, const Vec3& b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z;  // NOLINT(clang-diagnostic-float-equal)
}
[[nodiscard]] constexpr bool operator!=(const Vec3& a, const Vec3& b) noexcept {
    return !(a == b);
}

[[nodiscard]] inline bool almostEqual(const Vec3& a, const Vec3& b,
                                      double tolerance = kLinearTolerance) noexcept {
    return almostEqual(a.x, b.x, tolerance) && almostEqual(a.y, b.y, tolerance) &&
           almostEqual(a.z, b.z, tolerance);
}

[[nodiscard]] inline double distance(const Vec3& a, const Vec3& b) noexcept {
    return (a - b).length();
}

[[nodiscard]] constexpr Vec3 lerp(const Vec3& a, const Vec3& b, double t) noexcept {
    return a + (b - a) * t;
}

[[nodiscard]] constexpr Vec3 componentMin(const Vec3& a, const Vec3& b) noexcept {
    return {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z};
}
[[nodiscard]] constexpr Vec3 componentMax(const Vec3& a, const Vec3& b) noexcept {
    return {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z};
}

GK_EXPORT std::ostream& operator<<(std::ostream& os, const Vec3& v);

}  // namespace gk::math
