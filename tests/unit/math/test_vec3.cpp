#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <sstream>

#include "geokernel/math/Vec3.hpp"

using gk::math::Vec3;

TEST(Vec3, ArithmeticOperators) {
    constexpr Vec3 a{1, 2, 3};
    constexpr Vec3 b{4, 5, 6};
    static_assert(a + b == Vec3{5, 7, 9});
    static_assert(b - a == Vec3{3, 3, 3});
    static_assert(a * 2.0 == Vec3{2, 4, 6});
    static_assert(2.0 * a == Vec3{2, 4, 6});
    static_assert(-a == Vec3{-1, -2, -3});
    EXPECT_EQ(b / 2.0, (Vec3{2, 2.5, 3}));
}

TEST(Vec3, DotAndCrossFollowRightHandRule) {
    static_assert(Vec3::unitX().dot(Vec3::unitY()) == 0.0);
    static_assert(Vec3::unitX().cross(Vec3::unitY()) == Vec3::unitZ());
    static_assert(Vec3::unitY().cross(Vec3::unitZ()) == Vec3::unitX());
    static_assert(Vec3::unitZ().cross(Vec3::unitX()) == Vec3::unitY());
    EXPECT_DOUBLE_EQ((Vec3{1, 2, 3}.dot(Vec3{4, -5, 6})), 12.0);
}

TEST(Vec3, LengthAndNormalization) {
    const Vec3 v{3, 4, 12};
    EXPECT_DOUBLE_EQ(v.length(), 13.0);
    EXPECT_NEAR(v.normalized().length(), 1.0, 1e-15);
    EXPECT_EQ(Vec3::zero().normalized(), Vec3::zero()) << "zero vector must not produce NaN";
}

TEST(Vec3, FiniteAndZeroChecks) {
    EXPECT_TRUE((Vec3{1, 2, 3}.isFinite()));
    EXPECT_FALSE((Vec3{std::numeric_limits<double>::quiet_NaN(), 0, 0}.isFinite()));
    EXPECT_FALSE((Vec3{0, std::numeric_limits<double>::infinity(), 0}.isFinite()));
    EXPECT_TRUE((Vec3{1e-14, 0, 0}.isZero()));
    EXPECT_FALSE((Vec3{1e-6, 0, 0}.isZero()));
}

TEST(Vec3, HelpersAndStreaming) {
    EXPECT_TRUE(gk::math::almostEqual(Vec3{1, 1, 1}, Vec3{1 + 1e-12, 1, 1}));
    EXPECT_FALSE(gk::math::almostEqual(Vec3{1, 1, 1}, Vec3{1.001, 1, 1}));
    EXPECT_DOUBLE_EQ(gk::math::distance({0, 0, 0}, {0, 3, 4}), 5.0);
    EXPECT_EQ(gk::math::lerp({0, 0, 0}, {2, 4, 6}, 0.5), (Vec3{1, 2, 3}));
    EXPECT_EQ(gk::math::componentMin({1, 5, 3}, {4, 2, 6}), (Vec3{1, 2, 3}));
    EXPECT_EQ(gk::math::componentMax({1, 5, 3}, {4, 2, 6}), (Vec3{4, 5, 6}));
    EXPECT_DOUBLE_EQ((Vec3{7, 8, 9}[1]), 8.0);

    std::ostringstream os;
    os << Vec3{1, 2, 3};
    EXPECT_EQ(os.str(), "Vec3(1, 2, 3)");
}
