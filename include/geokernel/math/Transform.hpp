#pragma once
/// @file Transform.hpp
/// @brief Invertible affine 3D transform with a cached inverse.
///
/// Storing the inverse alongside the matrix makes normal transformation
/// (inverse-transpose) and world->local queries O(1) and guarantees a
/// Transform object is never singular: every construction path either
/// produces a valid inverse or fails loudly.

#include <optional>

#include "geokernel/Export.hpp"
#include "geokernel/math/Mat4.hpp"
#include "geokernel/math/Quat.hpp"
#include "geokernel/math/Vec3.hpp"

namespace gk::math {

class GK_EXPORT Transform {
public:
    /// Identity transform.
    Transform() noexcept = default;

    /// nullopt when `m` is not affine or not invertible.
    [[nodiscard]] static std::optional<Transform> fromMatrix(const Mat4& m) noexcept;

    [[nodiscard]] static Transform translation(const Vec3& t) noexcept;
    /// @throws std::invalid_argument if any scale component is ~0 (non-invertible).
    [[nodiscard]] static Transform scaling(const Vec3& s);
    [[nodiscard]] static Transform uniformScaling(double s);
    [[nodiscard]] static Transform rotation(const Quat& q) noexcept;
    [[nodiscard]] static Transform rotation(const Vec3& axis, double radians) noexcept;
    /// Scale, then rotate, then translate (the standard TRS order).
    [[nodiscard]] static Transform fromTRS(const Vec3& t, const Quat& r, const Vec3& s);

    /// Composition: (A * B)(p) == A(B(p)).
    [[nodiscard]] Transform operator*(const Transform& rhs) const noexcept;
    /// Fluent composition: a.then(b) applies a first, then b.
    [[nodiscard]] Transform then(const Transform& next) const noexcept { return next * *this; }

    [[nodiscard]] Transform inverse() const noexcept;

    [[nodiscard]] Vec3 applyToPoint(const Vec3& p) const noexcept;
    [[nodiscard]] Vec3 applyToVector(const Vec3& v) const noexcept;
    /// Normals transform with the inverse-transpose; result is re-normalised.
    [[nodiscard]] Vec3 applyToNormal(const Vec3& n) const noexcept;

    [[nodiscard]] const Mat4& matrix() const noexcept { return matrix_; }
    [[nodiscard]] const Mat4& inverseMatrix() const noexcept { return inverse_; }

    [[nodiscard]] bool isIdentity(double tolerance = kLinearTolerance) const noexcept {
        return matrix_.isIdentity(tolerance);
    }
    /// True when the transform flips handedness (negative determinant).
    [[nodiscard]] bool flipsOrientation() const noexcept { return matrix_.determinant() < 0.0; }

private:
    Transform(const Mat4& m, const Mat4& inv) noexcept : matrix_(m), inverse_(inv) {}

    Mat4 matrix_{};
    Mat4 inverse_{};
};

}  // namespace gk::math
