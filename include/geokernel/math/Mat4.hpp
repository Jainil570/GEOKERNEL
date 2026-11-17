#pragma once
/// @file Mat4.hpp
/// @brief 4x4 double matrix for homogeneous 3D transforms.
///
/// Conventions (documented once, used everywhere):
///  * Column vectors:   p' = M * p
///  * Row-major storage: element (row, col) at index row * 4 + col
///  * Translation lives in column 3: (0,3), (1,3), (2,3)
///  * Composition:      (A * B) applies B first, then A

#include <array>
#include <cstddef>
#include <iosfwd>
#include <optional>

#include "geokernel/Export.hpp"
#include "geokernel/math/Tolerance.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::math {

class GK_EXPORT Mat4 {
public:
    /// Identity by default — the safe neutral element for transforms.
    // clang-format off
    constexpr Mat4() noexcept
        : m_{1.0, 0.0, 0.0, 0.0,
             0.0, 1.0, 0.0, 0.0,
             0.0, 0.0, 1.0, 0.0,
             0.0, 0.0, 0.0, 1.0} {}
    // clang-format on

    /// Row-major element list.
    constexpr explicit Mat4(const std::array<double, 16>& rowMajor) noexcept : m_(rowMajor) {}

    [[nodiscard]] static constexpr Mat4 identity() noexcept { return Mat4{}; }
    [[nodiscard]] static constexpr Mat4 zero() noexcept { return Mat4{std::array<double, 16>{}}; }

    [[nodiscard]] static Mat4 translation(const Vec3& t) noexcept;
    [[nodiscard]] static Mat4 scaling(const Vec3& s) noexcept;
    [[nodiscard]] static Mat4 rotationX(double radians) noexcept;
    [[nodiscard]] static Mat4 rotationY(double radians) noexcept;
    [[nodiscard]] static Mat4 rotationZ(double radians) noexcept;
    /// Rotation about an arbitrary axis (Rodrigues). Identity if axis is ~zero.
    [[nodiscard]] static Mat4 rotation(const Vec3& axis, double radians) noexcept;

    [[nodiscard]] constexpr double operator()(std::size_t row, std::size_t col) const noexcept {
        return m_[row * 4 + col];
    }
    [[nodiscard]] constexpr double& operator()(std::size_t row, std::size_t col) noexcept {
        return m_[row * 4 + col];
    }

    [[nodiscard]] const std::array<double, 16>& data() const noexcept { return m_; }

    [[nodiscard]] Mat4 operator*(const Mat4& rhs) const noexcept;

    /// Applies the full homogeneous transform (perspective divide when w != 1).
    [[nodiscard]] Vec3 transformPoint(const Vec3& p) const noexcept;
    /// Applies only the upper-left 3x3 (directions ignore translation).
    [[nodiscard]] Vec3 transformVector(const Vec3& v) const noexcept;

    [[nodiscard]] Mat4 transposed() const noexcept;
    [[nodiscard]] double determinant() const noexcept;

    /// General 4x4 inverse via cofactors; nullopt if |det| < kEpsilon.
    [[nodiscard]] std::optional<Mat4> inverse() const noexcept;

    /// True when the last row is (0, 0, 0, 1).
    [[nodiscard]] bool isAffine(double tolerance = kEpsilon) const noexcept;

    [[nodiscard]] bool isIdentity(double tolerance = kLinearTolerance) const noexcept;

private:
    std::array<double, 16> m_;
};

[[nodiscard]] GK_EXPORT bool almostEqual(const Mat4& a, const Mat4& b,
                                         double tolerance = kLinearTolerance) noexcept;

GK_EXPORT std::ostream& operator<<(std::ostream& os, const Mat4& m);

}  // namespace gk::math
