#include <gtest/gtest.h>

#include <stdexcept>

#include "geokernel/math/Transform.hpp"

using gk::math::almostEqual;
using gk::math::degToRad;
using gk::math::Mat4;
using gk::math::Quat;
using gk::math::Transform;
using gk::math::Vec3;

TEST(Transform, DefaultIsIdentity) {
    const Transform t;
    EXPECT_TRUE(t.isIdentity());
    EXPECT_EQ(t.applyToPoint({1, 2, 3}), (Vec3{1, 2, 3}));
}

TEST(Transform, CachedInverseUndoesTransform) {
    const Transform t = Transform::fromTRS({5, -1, 2}, Quat::fromAxisAngle({1, 1, 1}, 0.9), {2, 3, 4});
    const Vec3 p{0.3, -7.0, 11.0};
    EXPECT_TRUE(almostEqual(t.inverse().applyToPoint(t.applyToPoint(p)), p, 1e-12));
    EXPECT_TRUE((t.matrix() * t.inverseMatrix()).isIdentity(1e-12));
}

TEST(Transform, ThenComposesInApplicationOrder) {
    const Transform rotate = Transform::rotation(Vec3::unitZ(), degToRad(90));
    const Transform move = Transform::translation({10, 0, 0});
    // rotate first, then move: (1,0,0) -> (0,1,0) -> (10,1,0)
    EXPECT_TRUE(almostEqual(rotate.then(move).applyToPoint(Vec3::unitX()), Vec3{10, 1, 0}));
    // move first, then rotate: (1,0,0) -> (11,0,0) -> (0,11,0)
    EXPECT_TRUE(almostEqual(move.then(rotate).applyToPoint(Vec3::unitX()), Vec3{0, 11, 0}));
    // Composite inverse is consistent.
    const Transform c = rotate.then(move);
    EXPECT_TRUE((c * c.inverse()).isIdentity(1e-12));
}

TEST(Transform, NormalsUseInverseTranspose) {
    // Non-uniform scale: a 45° plane normal must not simply be scaled.
    const Transform s = Transform::scaling({2, 1, 1});
    const Vec3 n = Vec3{1, 1, 0}.normalized();
    const Vec3 tangent{1, -1, 0};  // lies in the plane, perpendicular to n
    const Vec3 nT = s.applyToNormal(n);
    EXPECT_NEAR(nT.dot(s.applyToVector(tangent)), 0.0, 1e-12) << "normal must stay perpendicular";
    EXPECT_NEAR(nT.length(), 1.0, 1e-12);
}

TEST(Transform, FromMatrixRejectsSingularAndProjective) {
    EXPECT_TRUE(Transform::fromMatrix(Mat4::rotationX(0.3)).has_value());
    EXPECT_FALSE(Transform::fromMatrix(Mat4::scaling({0, 1, 1})).has_value());
    Mat4 projective;
    projective(3, 2) = 1.0;
    EXPECT_FALSE(Transform::fromMatrix(projective).has_value());
}

TEST(Transform, ZeroScaleThrowsAndMirrorFlipsOrientation) {
    EXPECT_THROW(static_cast<void>(Transform::scaling({1, 0, 1})), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(Transform::uniformScaling(0.0)), std::invalid_argument);
    EXPECT_TRUE(Transform::scaling({-1, 1, 1}).flipsOrientation());
    EXPECT_FALSE(Transform::rotation(Vec3::unitY(), 2.0).flipsOrientation());
}
