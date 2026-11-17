#include "geokernel/geometry/AABB.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gk::geometry {
namespace {
constexpr double kInf = std::numeric_limits<double>::infinity();
}  // namespace

AABB::AABB() noexcept : min_{kInf, kInf, kInf}, max_{-kInf, -kInf, -kInf} {}

AABB::AABB(const Vec3& p, const Vec3& q) noexcept
    : min_(math::componentMin(p, q)), max_(math::componentMax(p, q)) {}

AABB AABB::fromPoints(const std::vector<Vec3>& points) noexcept {
    AABB box;
    for (const auto& p : points) {
        box.expand(p);
    }
    return box;
}

bool AABB::isEmpty() const noexcept {
    return min_.x > max_.x || min_.y > max_.y || min_.z > max_.z;
}

void AABB::expand(const Vec3& p) noexcept {
    min_ = math::componentMin(min_, p);
    max_ = math::componentMax(max_, p);
}

void AABB::expand(const AABB& other) noexcept {
    if (other.isEmpty()) {
        return;
    }
    min_ = math::componentMin(min_, other.min_);
    max_ = math::componentMax(max_, other.max_);
}

Vec3 AABB::center() const noexcept {
    return isEmpty() ? Vec3::zero() : (min_ + max_) * 0.5;
}

Vec3 AABB::extent() const noexcept {
    return isEmpty() ? Vec3::zero() : max_ - min_;
}

double AABB::surfaceArea() const noexcept {
    const Vec3 e = extent();
    return 2.0 * (e.x * e.y + e.y * e.z + e.z * e.x);
}

double AABB::volume() const noexcept {
    const Vec3 e = extent();
    return e.x * e.y * e.z;
}

std::array<Vec3, 8> AABB::corners() const noexcept {
    return {{{min_.x, min_.y, min_.z},
             {max_.x, min_.y, min_.z},
             {min_.x, max_.y, min_.z},
             {max_.x, max_.y, min_.z},
             {min_.x, min_.y, max_.z},
             {max_.x, min_.y, max_.z},
             {min_.x, max_.y, max_.z},
             {max_.x, max_.y, max_.z}}};
}

bool AABB::contains(const Vec3& p, double tolerance) const noexcept {
    return p.x >= min_.x - tolerance && p.x <= max_.x + tolerance &&  //
           p.y >= min_.y - tolerance && p.y <= max_.y + tolerance &&  //
           p.z >= min_.z - tolerance && p.z <= max_.z + tolerance;
}

bool AABB::intersects(const AABB& other) const noexcept {
    if (isEmpty() || other.isEmpty()) {
        return false;
    }
    return min_.x <= other.max_.x && max_.x >= other.min_.x &&  //
           min_.y <= other.max_.y && max_.y >= other.min_.y &&  //
           min_.z <= other.max_.z && max_.z >= other.min_.z;
}

std::optional<std::pair<double, double>> AABB::intersect(const Ray& ray) const noexcept {
    if (isEmpty()) {
        return std::nullopt;
    }
    double tEnter = ray.tMin;
    double tExit = ray.tMax;
    for (std::size_t axis = 0; axis < 3; ++axis) {
        const double o = ray.origin[axis];
        const double d = ray.direction[axis];
        const double lo = min_[axis];
        const double hi = max_[axis];
        if (std::abs(d) < math::kEpsilon) {
            // Parallel to this slab: must already be inside it.
            if (o < lo || o > hi) {
                return std::nullopt;
            }
            continue;
        }
        const double inv = 1.0 / d;
        double tNear = (lo - o) * inv;
        double tFar = (hi - o) * inv;
        if (tNear > tFar) {
            std::swap(tNear, tFar);
        }
        tEnter = std::max(tEnter, tNear);
        tExit = std::min(tExit, tFar);
        if (tEnter > tExit) {
            return std::nullopt;
        }
    }
    return std::make_pair(tEnter, tExit);
}

AABB AABB::transformed(const math::Transform& xf) const noexcept {
    if (isEmpty()) {
        return {};
    }
    AABB out;
    for (const auto& corner : corners()) {
        out.expand(xf.applyToPoint(corner));
    }
    return out;
}

}  // namespace gk::geometry
