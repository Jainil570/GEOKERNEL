#include "geokernel/math/Quat.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>

namespace gk::math {

Quat Quat::fromAxisAngle(const Vec3& axis, double radians) noexcept {
    const Vec3 a = axis.normalized();
    if (a.isZero()) {
        return identity();
    }
    const double half = 0.5 * radians;
    const double s = std::sin(half);
    return {std::cos(half), a.x * s, a.y * s, a.z * s};
}

double Quat::norm() const noexcept {
    return std::sqrt(normSquared());
}

Quat Quat::normalized() const noexcept {
    const double n = norm();
    if (n < kEpsilon) {
        return identity();
    }
    return {w / n, x / n, y / n, z / n};
}

Quat Quat::inverse() const noexcept {
    const double n2 = normSquared();
    if (n2 < kEpsilon) {
        return identity();
    }
    const Quat c = conjugate();
    return {c.w / n2, c.x / n2, c.y / n2, c.z / n2};
}

Vec3 Quat::rotate(const Vec3& v) const noexcept {
    // v' = v + w*t + u x t,  with t = 2 (u x v)  — 15 mul vs 28 for q v q*.
    const Vec3 u{x, y, z};
    const Vec3 t = 2.0 * u.cross(v);
    return v + w * t + u.cross(t);
}

Mat4 Quat::toMatrix() const noexcept {
    const Quat q = normalized();
    const double xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    const double xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
    const double wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

    Mat4 r;
    r(0, 0) = 1.0 - 2.0 * (yy + zz);
    r(0, 1) = 2.0 * (xy - wz);
    r(0, 2) = 2.0 * (xz + wy);
    r(1, 0) = 2.0 * (xy + wz);
    r(1, 1) = 1.0 - 2.0 * (xx + zz);
    r(1, 2) = 2.0 * (yz - wx);
    r(2, 0) = 2.0 * (xz - wy);
    r(2, 1) = 2.0 * (yz + wx);
    r(2, 2) = 1.0 - 2.0 * (xx + yy);
    return r;
}

Quat Quat::slerp(const Quat& a, const Quat& b, double t) noexcept {
    t = std::clamp(t, 0.0, 1.0);
    Quat end = b;
    double cosTheta = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    if (cosTheta < 0.0) {  // take the shortest arc
        end = {-b.w, -b.x, -b.y, -b.z};
        cosTheta = -cosTheta;
    }

    double wa = 1.0 - t;
    double wb = t;
    if (cosTheta < 1.0 - 1e-6) {  // otherwise nearly parallel: linear blend is exact enough
        const double theta = std::acos(cosTheta);
        const double sinTheta = std::sin(theta);
        wa = std::sin((1.0 - t) * theta) / sinTheta;
        wb = std::sin(t * theta) / sinTheta;
    }
    return Quat{wa * a.w + wb * end.w, wa * a.x + wb * end.x, wa * a.y + wb * end.y, wa * a.z + wb * end.z}
        .normalized();
}

bool almostEqualRotation(const Quat& a, const Quat& b, double tolerance) noexcept {
    const Quat na = a.normalized();
    const Quat nb = b.normalized();
    const auto same = [tolerance](const Quat& p, const Quat& q) {
        return almostEqual(p.w, q.w, tolerance) && almostEqual(p.x, q.x, tolerance) &&
               almostEqual(p.y, q.y, tolerance) && almostEqual(p.z, q.z, tolerance);
    };
    return same(na, nb) || same(na, Quat{-nb.w, -nb.x, -nb.y, -nb.z});
}

std::ostream& operator<<(std::ostream& os, const Quat& q) {
    return os << "Quat(w=" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ')';
}

}  // namespace gk::math
