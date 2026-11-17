#include <gtest/gtest.h>

#include <sstream>

#include "geokernel/math/Mat4.hpp"

using gk::math::almostEqual;
using gk::math::degToRad;
using gk::math::Mat4;
using gk::math::Vec3;

TEST(Mat4, DefaultIsIdentity) {
    constexpr Mat4 m;
    static_assert(m(0, 0) == 1.0 && m(1, 1) == 1.0 && m(3, 3) == 1.0 && m(0, 1) == 0.0);
    EXPECT_TRUE(m.isIdentity());
    EXPECT_TRUE(m.isAffine());
    EXPECT_DOUBLE_EQ(m.determinant(), 1.0);
}

TEST(Mat4, TranslationMovesPointsButNotVectors) {
    const Mat4 t = Mat4::translation({1, 2, 3});
    EXPECT_EQ(t.transformPoint({1, 1, 1}), (Vec3{2, 3, 4}));
    EXPECT_EQ(t.transformVector({1, 1, 1}), (Vec3{1, 1, 1}));
}

TEST(Mat4, RotationZQuarterTurnMapsXToY) {
    const Mat4 r = Mat4::rotationZ(degToRad(90));
    EXPECT_TRUE(almostEqual(r.transformVector(Vec3::unitX()), Vec3::unitY()));
    EXPECT_NEAR(r.determinant(), 1.0, 1e-12) << "rotations preserve volume";
}

TEST(Mat4, ArbitraryAxisRotationMatchesPrincipalAxes) {
    for (const double deg : {-120.0, 30.0, 45.0, 200.0}) {
        EXPECT_TRUE(
            almostEqual(Mat4::rotation(Vec3::unitX(), degToRad(deg)), Mat4::rotationX(degToRad(deg))));
        EXPECT_TRUE(almostEqual(Mat4::rotation({0, 5, 0}, degToRad(deg)), Mat4::rotationY(degToRad(deg))));
    }
    EXPECT_TRUE(Mat4::rotation(Vec3::zero(), 1.0).isIdentity()) << "degenerate axis -> identity";
}

TEST(Mat4, MultiplicationComposesRightToLeft) {
    const Mat4 t = Mat4::translation({10, 0, 0});
    const Mat4 s = Mat4::scaling({2, 2, 2});
    // (t * s) scales first, then translates.
    EXPECT_EQ((t * s).transformPoint({1, 0, 0}), (Vec3{12, 0, 0}));
    EXPECT_EQ((s * t).transformPoint({1, 0, 0}), (Vec3{22, 0, 0}));
}

TEST(Mat4, InverseOfGeneralAffineMatrix) {
    const Mat4 m =
        Mat4::translation({1, -2, 3}) * Mat4::rotation({1, 1, 0}, 0.7) * Mat4::scaling({2, 3, 0.5});
    const auto inv = m.inverse();
    ASSERT_TRUE(inv.has_value());
    EXPECT_TRUE((m * *inv).isIdentity(1e-12));
    EXPECT_TRUE((*inv * m).isIdentity(1e-12));
    EXPECT_NEAR(m.determinant() * inv->determinant(), 1.0, 1e-12);
}

TEST(Mat4, SingularMatrixHasNoInverse) {
    EXPECT_FALSE(Mat4::scaling({1, 0, 1}).inverse().has_value());
    EXPECT_FALSE(Mat4::zero().inverse().has_value());
}

TEST(Mat4, TransposeAndPerspectiveDivide) {
    const Mat4 m = Mat4::translation({1, 2, 3});
    EXPECT_DOUBLE_EQ(m.transposed()(3, 0), 1.0);
    EXPECT_TRUE(almostEqual(m.transposed().transposed(), m));

    Mat4 p;  // w' = 2 for every point -> coordinates halved
    p(3, 3) = 2.0;
    EXPECT_FALSE(p.isAffine());
    EXPECT_EQ(p.transformPoint({2, 4, 6}), (Vec3{1, 2, 3}));

    std::ostringstream os;
    os << Mat4::identity();
    EXPECT_EQ(os.str().rfind("Mat4[1, 0, 0, 0;", 0), 0U);
}
