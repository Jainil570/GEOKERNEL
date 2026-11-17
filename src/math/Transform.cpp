#include "geokernel/math/Transform.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

namespace gk::math {

std::optional<Transform> Transform::fromMatrix(const Mat4& m) noexcept {
    if (!m.isAffine()) {
        return std::nullopt;
    }
    const auto inv = m.inverse();
    if (!inv) {
        return std::nullopt;
    }
    return Transform{m, *inv};
}

Transform Transform::translation(const Vec3& t) noexcept {
    return Transform{Mat4::translation(t), Mat4::translation(-t)};
}

Transform Transform::scaling(const Vec3& s) {
    if (std::abs(s.x) < kEpsilon || std::abs(s.y) < kEpsilon || std::abs(s.z) < kEpsilon) {
        throw std::invalid_argument("Transform::scaling: zero scale component is not invertible");
    }
    return Transform{Mat4::scaling(s), Mat4::scaling({1.0 / s.x, 1.0 / s.y, 1.0 / s.z})};
}

Transform Transform::uniformScaling(double s) {
    return scaling({s, s, s});
}

Transform Transform::rotation(const Quat& q) noexcept {
    const Mat4 m = q.toMatrix();
    return Transform{m, m.transposed()};  // orthonormal: inverse == transpose
}

Transform Transform::rotation(const Vec3& axis, double radians) noexcept {
    const Mat4 m = Mat4::rotation(axis, radians);
    return Transform{m, m.transposed()};
}

Transform Transform::fromTRS(const Vec3& t, const Quat& r, const Vec3& s) {
    return translation(t) * rotation(r) * scaling(s);
}

Transform Transform::operator*(const Transform& rhs) const noexcept {
    // (A B)^-1 = B^-1 A^-1
    return Transform{matrix_ * rhs.matrix_, rhs.inverse_ * inverse_};
}

Transform Transform::inverse() const noexcept {
    return Transform{inverse_, matrix_};
}

Vec3 Transform::applyToPoint(const Vec3& p) const noexcept {
    return matrix_.transformPoint(p);
}

Vec3 Transform::applyToVector(const Vec3& v) const noexcept {
    return matrix_.transformVector(v);
}

Vec3 Transform::applyToNormal(const Vec3& n) const noexcept {
    // Multiply by (M^-1)^T without materialising the transpose.
    const Mat4& inv = inverse_;
    const Vec3 r{inv(0, 0) * n.x + inv(1, 0) * n.y + inv(2, 0) * n.z,
                 inv(0, 1) * n.x + inv(1, 1) * n.y + inv(2, 1) * n.z,
                 inv(0, 2) * n.x + inv(1, 2) * n.y + inv(2, 2) * n.z};
    return r.normalized();
}

}  // namespace gk::math
