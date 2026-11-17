#include <gtest/gtest.h>

#include <cmath>

#include "geokernel/geometry/AABB.hpp"

using gk::geometry::AABB;
using gk::geometry::Ray;
using gk::math::Transform;
using gk::math::Vec3;

TEST(AABB, DefaultIsEmptyAndExpandWorks) {
    AABB box;
    EXPECT_TRUE(box.isEmpty());
    EXPECT_EQ(box.extent(), Vec3::zero());
    box.expand({1, 2, 3});
    EXPECT_FALSE(box.isEmpty());
    box.expand({-1, 0, 5});
    EXPECT_EQ(box.min(), (Vec3{-1, 0, 3}));
    EXPECT_EQ(box.max(), (Vec3{1, 2, 5}));
    box.expand(AABB{});  // expanding by an empty box is a no-op
    EXPECT_EQ(box.min(), (Vec3{-1, 0, 3}));
}

TEST(AABB, MeasuresOfUnitCube) {
    const AABB box({1, 1, 1}, {0, 0, 0});  // corner order does not matter
    EXPECT_EQ(box.center(), (Vec3{0.5, 0.5, 0.5}));
    EXPECT_DOUBLE_EQ(box.volume(), 1.0);
    EXPECT_DOUBLE_EQ(box.surfaceArea(), 6.0);
    EXPECT_EQ(box.corners().size(), 8U);
    EXPECT_TRUE(box.contains({0.5, 0.5, 0.5}));
    EXPECT_TRUE(box.contains({1, 1, 1}));
    EXPECT_FALSE(box.contains({1.1, 0.5, 0.5}));
}

TEST(AABB, OverlapTest) {
    const AABB a({0, 0, 0}, {1, 1, 1});
    EXPECT_TRUE(a.intersects(AABB({0.5, 0.5, 0.5}, {2, 2, 2})));
    EXPECT_TRUE(a.intersects(AABB({1, 0, 0}, {2, 1, 1}))) << "touching faces overlap";
    EXPECT_FALSE(a.intersects(AABB({1.5, 0, 0}, {2, 1, 1})));
    EXPECT_FALSE(a.intersects(AABB{}));
}

TEST(AABB, RaySlabTestHitAndMiss) {
    const AABB box({0, 0, 0}, {1, 1, 1});
    const auto hit = box.intersect(Ray{{-1, 0.5, 0.5}, {1, 0, 0}});
    ASSERT_TRUE(hit.has_value());
    EXPECT_DOUBLE_EQ(hit->first, 1.0);
    EXPECT_DOUBLE_EQ(hit->second, 2.0);

    EXPECT_FALSE(box.intersect(Ray{{-1, 2, 0.5}, {1, 0, 0}}).has_value()) << "passes above";
    EXPECT_FALSE(box.intersect(Ray{{-1, 0.5, 0.5}, {-1, 0, 0}}).has_value()) << "points away";
    EXPECT_FALSE(box.intersect(Ray{{-1, 0.5, 0.5}, {1, 0, 0}, 0.0, 0.5}).has_value()) << "too short";
    EXPECT_FALSE(AABB{}.intersect(Ray{}).has_value());
}

TEST(AABB, RayStartingInsideAndAxisParallel) {
    const AABB box({0, 0, 0}, {1, 1, 1});
    const auto inside = box.intersect(Ray{{0.5, 0.5, 0.5}, {0, 0, 1}});
    ASSERT_TRUE(inside.has_value());
    EXPECT_DOUBLE_EQ(inside->first, 0.0);  // clipped to ray.tMin
    EXPECT_DOUBLE_EQ(inside->second, 0.5);
}

TEST(AABB, TransformedBoundsRotatedBox) {
    const AABB box({-1, -1, -1}, {1, 1, 1});
    const AABB rotated = box.transformed(Transform::rotation(Vec3::unitZ(), gk::math::degToRad(45)));
    EXPECT_NEAR(rotated.max().x, std::sqrt(2.0), 1e-12);
    EXPECT_NEAR(rotated.max().z, 1.0, 1e-12);
    EXPECT_TRUE(AABB{}.transformed(Transform{}).isEmpty());
}
