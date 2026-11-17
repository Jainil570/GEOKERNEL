#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

#include "TestHelpers.hpp"
#include "geokernel/geometry/Mesh.hpp"

using gk::geometry::Mesh;
using gk::geometry::Triangle;
using gk::math::Transform;
using gk::math::Vec3;

TEST(Triangle, BasicMeasures) {
    const Triangle t = gk::test::makeXYTriangle();
    EXPECT_DOUBLE_EQ(t.area(), 0.5);
    EXPECT_EQ(t.normal(), Vec3::unitZ());
    EXPECT_EQ(t.pointAt(0, 0), t.a);
    EXPECT_EQ(t.pointAt(1, 0), t.b);
    EXPECT_EQ(t.pointAt(0, 1), t.c);
    EXPECT_FALSE(t.isDegenerate());
    EXPECT_TRUE((Triangle{{0, 0, 0}, {1, 1, 1}, {2, 2, 2}}.isDegenerate())) << "collinear";
}

TEST(Mesh, IncrementalBuildMaintainsBounds) {
    Mesh m;
    EXPECT_TRUE(m.empty());
    const auto a = m.addVertex({0, 0, 0});
    const auto b = m.addVertex({2, 0, 0});
    const auto c = m.addVertex({0, 3, -1});
    m.addFace(a, b, c);
    EXPECT_EQ(m.faceCount(), 1U);
    EXPECT_EQ(m.bounds().min(), (Vec3{0, 0, -1}));
    EXPECT_EQ(m.bounds().max(), (Vec3{2, 3, 0}));
    EXPECT_THROW(m.addFace(a, b, 99), std::out_of_range);
    EXPECT_THROW(static_cast<void>(m.triangle(5)), std::out_of_range);
}

TEST(Mesh, UnitCubeSurfaceAreaAndNormals) {
    const Mesh cube = gk::test::makeUnitCube();
    EXPECT_EQ(cube.vertexCount(), 8U);
    EXPECT_EQ(cube.faceCount(), 12U);
    EXPECT_DOUBLE_EQ(cube.surfaceArea(), 6.0);
    EXPECT_EQ(cube.countDegenerateFaces(), 0U);
    EXPECT_TRUE(cube.validate().ok());

    // Every face normal must point away from the cube centre (outward winding).
    const Vec3 centre = cube.bounds().center();
    const auto normals = cube.computeFaceNormals();
    for (std::size_t i = 0; i < cube.faceCount(); ++i) {
        EXPECT_GT(normals[i].dot(cube.triangle(i).centroid() - centre), 0.0) << "face " << i;
    }
    // Corner vertex normals point diagonally outward.
    const auto vn = cube.computeVertexNormals();
    EXPECT_TRUE(gk::math::almostEqual(vn[7], Vec3{1, 1, 1}.normalized(), 1e-12));
    EXPECT_GT(cube.memoryFootprint(), 0U);
}

TEST(Mesh, ValidateDetectsBadIndicesAndNonFiniteCoordinates) {
    Mesh badIndex({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}, {{0, 1, 3}});
    const auto s1 = badIndex.validate();
    ASSERT_FALSE(s1.ok());
    EXPECT_EQ(s1.error().code, gk::core::ErrorCode::InvalidArgument);

    Mesh nan({{0, 0, 0}, {std::numeric_limits<double>::quiet_NaN(), 0, 0}, {0, 1, 0}}, {{0, 1, 2}});
    EXPECT_FALSE(nan.validate().ok());
}

TEST(Mesh, DegenerateFacesAreCounted) {
    Mesh m({{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {0, 1, 0}}, {{0, 1, 2}, {0, 0, 3}, {0, 1, 3}});
    EXPECT_EQ(m.countDegenerateFaces(), 2U);  // collinear + repeated index
}

TEST(Mesh, TransformMovesVerticesAndBounds) {
    Mesh cube = gk::test::makeUnitCube();
    cube.transform(Transform::translation({10, 0, 0}).then(Transform::uniformScaling(2.0)));
    EXPECT_EQ(cube.bounds().min(), (Vec3{20, 0, 0}));
    EXPECT_EQ(cube.bounds().max(), (Vec3{22, 2, 2}));
    EXPECT_DOUBLE_EQ(cube.surfaceArea(), 24.0);
}

TEST(Mesh, MirrorTransformPreservesOutwardWinding) {
    const Mesh mirrored = gk::test::makeUnitCube().transformed(Transform::scaling({-1, 1, 1}));
    const Vec3 centre = mirrored.bounds().center();
    const auto normals = mirrored.computeFaceNormals();
    for (std::size_t i = 0; i < mirrored.faceCount(); ++i) {
        EXPECT_GT(normals[i].dot(mirrored.triangle(i).centroid() - centre), 0.0) << "face " << i;
    }
}

TEST(Mesh, ClearAndName) {
    Mesh cube = gk::test::makeUnitCube();
    EXPECT_EQ(cube.name(), "unit_cube");
    cube.setName("renamed");
    EXPECT_EQ(cube.name(), "renamed");
    cube.clear();
    EXPECT_TRUE(cube.empty());
    EXPECT_TRUE(cube.bounds().isEmpty());
}
