#include "geokernel/math/Mat4.hpp"

#include <cmath>
#include <ostream>

namespace gk::math {
namespace {

/// Adjugate (transposed cofactor matrix) of a flat 4x4 matrix. The formula is
/// layout-agnostic: inverse(transpose(M)) == transpose(inverse(M)).
std::array<double, 16> adjugate(const std::array<double, 16>& m) noexcept {
    std::array<double, 16> inv{};
    // clang-format off  (hand-aligned so each cofactor can be checked against the reference)
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
             m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
              m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
             m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
             m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
             m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
              m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    // clang-format on
    return inv;
}

double determinantFromAdjugate(const std::array<double, 16>& m, const std::array<double, 16>& adj) noexcept {
    return m[0] * adj[0] + m[1] * adj[4] + m[2] * adj[8] + m[3] * adj[12];
}

}  // namespace

Mat4 Mat4::translation(const Vec3& t) noexcept {
    Mat4 r;
    r(0, 3) = t.x;
    r(1, 3) = t.y;
    r(2, 3) = t.z;
    return r;
}

Mat4 Mat4::scaling(const Vec3& s) noexcept {
    Mat4 r;
    r(0, 0) = s.x;
    r(1, 1) = s.y;
    r(2, 2) = s.z;
    return r;
}

Mat4 Mat4::rotationX(double radians) noexcept {
    return rotation(Vec3::unitX(), radians);
}
Mat4 Mat4::rotationY(double radians) noexcept {
    return rotation(Vec3::unitY(), radians);
}
Mat4 Mat4::rotationZ(double radians) noexcept {
    return rotation(Vec3::unitZ(), radians);
}

Mat4 Mat4::rotation(const Vec3& axis, double radians) noexcept {
    const Vec3 a = axis.normalized();
    if (a.isZero()) {
        return identity();
    }
    const double c = std::cos(radians);
    const double s = std::sin(radians);
    const double t = 1.0 - c;

    Mat4 r;
    r(0, 0) = c + t * a.x * a.x;
    r(0, 1) = t * a.x * a.y - s * a.z;
    r(0, 2) = t * a.x * a.z + s * a.y;
    r(1, 0) = t * a.y * a.x + s * a.z;
    r(1, 1) = c + t * a.y * a.y;
    r(1, 2) = t * a.y * a.z - s * a.x;
    r(2, 0) = t * a.z * a.x - s * a.y;
    r(2, 1) = t * a.z * a.y + s * a.x;
    r(2, 2) = c + t * a.z * a.z;
    return r;
}

Mat4 Mat4::operator*(const Mat4& rhs) const noexcept {
    Mat4 r = zero();
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t col = 0; col < 4; ++col) {
            double sum = 0.0;
            for (std::size_t k = 0; k < 4; ++k) {
                sum += (*this)(row, k) * rhs(k, col);
            }
            r(row, col) = sum;
        }
    }
    return r;
}

Vec3 Mat4::transformPoint(const Vec3& p) const noexcept {
    const auto& m = *this;
    const double x = m(0, 0) * p.x + m(0, 1) * p.y + m(0, 2) * p.z + m(0, 3);
    const double y = m(1, 0) * p.x + m(1, 1) * p.y + m(1, 2) * p.z + m(1, 3);
    const double z = m(2, 0) * p.x + m(2, 1) * p.y + m(2, 2) * p.z + m(2, 3);
    const double w = m(3, 0) * p.x + m(3, 1) * p.y + m(3, 2) * p.z + m(3, 3);
    if (std::abs(w - 1.0) > kEpsilon && std::abs(w) > kEpsilon) {
        return {x / w, y / w, z / w};
    }
    return {x, y, z};  // affine (w == 1) or point at infinity (w == 0)
}

Vec3 Mat4::transformVector(const Vec3& v) const noexcept {
    const auto& m = *this;
    return {m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z, m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z,
            m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z};
}

Mat4 Mat4::transposed() const noexcept {
    Mat4 r;
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t col = 0; col < 4; ++col) {
            r(row, col) = (*this)(col, row);
        }
    }
    return r;
}

double Mat4::determinant() const noexcept {
    return determinantFromAdjugate(m_, adjugate(m_));
}

std::optional<Mat4> Mat4::inverse() const noexcept {
    const auto adj = adjugate(m_);
    const double det = determinantFromAdjugate(m_, adj);
    if (std::abs(det) < kEpsilon || !std::isfinite(det)) {
        return std::nullopt;
    }
    const double invDet = 1.0 / det;
    std::array<double, 16> out{};
    for (std::size_t i = 0; i < 16; ++i) {
        out[i] = adj[i] * invDet;
    }
    return Mat4{out};
}

bool Mat4::isAffine(double tolerance) const noexcept {
    const auto& m = *this;
    return std::abs(m(3, 0)) <= tolerance && std::abs(m(3, 1)) <= tolerance &&
           std::abs(m(3, 2)) <= tolerance && std::abs(m(3, 3) - 1.0) <= tolerance;
}

bool Mat4::isIdentity(double tolerance) const noexcept {
    return almostEqual(*this, identity(), tolerance);
}

bool almostEqual(const Mat4& a, const Mat4& b, double tolerance) noexcept {
    for (std::size_t i = 0; i < 16; ++i) {
        if (!almostEqual(a.data()[i], b.data()[i], tolerance)) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const Mat4& m) {
    os << "Mat4[";
    for (std::size_t row = 0; row < 4; ++row) {
        os << (row == 0 ? "" : "; ");
        for (std::size_t col = 0; col < 4; ++col) {
            os << (col == 0 ? "" : ", ") << m(row, col);
        }
    }
    return os << ']';
}

}  // namespace gk::math
