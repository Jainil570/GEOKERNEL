#include <gtest/gtest.h>

#include <sstream>

#include "geokernel/math/Quat.hpp"

using gk::math::almostEqual;
using gk::math::almostEqualRotation;
using gk::math::degToRad;
using gk::math::Mat4;
using gk::math::Quat;
using gk::math::Vec3;

TEST(Quat, AxisAngleRotatesVectors) {
    const Quat q = Quat::fromAxisAngle(Vec3::unitZ(), degToRad(90));
    EXPECT_TRUE(almostEqual(q.rotate(Vec3::unitX()), Vec3::unitY()));
    EXPECT_NEAR(q.norm(), 1.0, 1e-15);
    EXPECT_TRUE(almostEqualRotation(Quat::fromAxisAngle(Vec3::zero(), 1.0), Quat::identity()));
}

TEST(Quat, MatrixAgreesWithDirectRotation) {
    const Quat q = Quat::fromAxisAngle({1, 2, 3}, 1.234);
    const Vec3 v{-0.3, 4.0, 2.5};
    EXPECT_TRUE(almostEqual(q.toMatrix().transformVector(v), q.rotate(v), 1e-12));
    EXPECT_TRUE(almostEqual(q.toMatrix(), Mat4::rotation({1, 2, 3}, 1.234), 1e-12));
}

TEST(Quat, CompositionAndInverse) {
    const Quat a = Quat::fromAxisAngle(Vec3::unitX(), 0.4);
    const Quat b = Quat::fromAxisAngle(Vec3::unitY(), -1.1);
    const Vec3 v{1, 2, 3};
    // (a * b) applies b first.
    EXPECT_TRUE(almostEqual((a * b).rotate(v), a.rotate(b.rotate(v)), 1e-12));
    EXPECT_TRUE(almostEqualRotation(a * a.inverse(), Quat::identity()));
    EXPECT_TRUE(almostEqualRotation(Quat{0, 0, 0, 0}.inverse(), Quat::identity()));
}

TEST(Quat, NegatedQuaternionIsSameRotation) {
    const Quat q = Quat::fromAxisAngle({0, 1, 1}, 0.8);
    EXPECT_TRUE(almostEqualRotation(q, Quat{-q.w, -q.x, -q.y, -q.z}));
    EXPECT_FALSE(almostEqualRotation(q, Quat::identity()));
}

TEST(Quat, SlerpEndpointsAndMidpoint) {
    const Quat a = Quat::identity();
    const Quat b = Quat::fromAxisAngle(Vec3::unitZ(), degToRad(90));
    EXPECT_TRUE(almostEqualRotation(Quat::slerp(a, b, 0.0), a));
    EXPECT_TRUE(almostEqualRotation(Quat::slerp(a, b, 1.0), b));
    EXPECT_TRUE(
        almostEqualRotation(Quat::slerp(a, b, 0.5), Quat::fromAxisAngle(Vec3::unitZ(), degToRad(45))));
    // Shortest arc: interpolating to -b must give the same result as to b.
    EXPECT_TRUE(
        almostEqualRotation(Quat::slerp(a, Quat{-b.w, -b.x, -b.y, -b.z}, 0.5), Quat::slerp(a, b, 0.5)));
    // Nearly identical inputs take the linear path without NaNs.
    EXPECT_TRUE(almostEqualRotation(Quat::slerp(a, a, 0.3), a));

    std::ostringstream os;
    os << Quat::identity();
    EXPECT_EQ(os.str(), "Quat(w=1, 0, 0, 0)");
}
