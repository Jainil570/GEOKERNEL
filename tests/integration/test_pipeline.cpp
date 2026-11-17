/// End-to-end tests over real files in tests/data: load -> transform ->
/// raycast, exercising every layer through public APIs only.

#include <gtest/gtest.h>

#include <memory>

#include "TestHelpers.hpp"
#include "geokernel/geokernel.hpp"

using gk::algorithms::MeshRaycaster;
using gk::geometry::Mesh;
using gk::geometry::Ray;
using gk::math::Transform;
using gk::math::Vec3;

TEST(Pipeline, AsciiStlAndObjFilesDescribeTheSameCube) {
    const auto stl = gk::loadMesh(gk::test::dataFile("cube_ascii.stl"));
    const auto obj = gk::loadMesh(gk::test::dataFile("cube.obj"));
    ASSERT_TRUE(stl.ok()) << stl.error().toString();
    ASSERT_TRUE(obj.ok()) << obj.error().toString();

    EXPECT_EQ(stl.value()->name(), "cube");
    EXPECT_EQ(obj.value()->name(), "cube");
    EXPECT_EQ(stl.value()->vertexCount(), 8U);
    EXPECT_EQ(obj.value()->vertexCount(), 8U);
    EXPECT_EQ(stl.value()->faceCount(), 12U);
    EXPECT_EQ(obj.value()->faceCount(), 12U) << "6 quads fan-triangulated";
    EXPECT_DOUBLE_EQ(stl.value()->surfaceArea(), obj.value()->surfaceArea());
    EXPECT_EQ(stl.value()->bounds().min(), obj.value()->bounds().min());
    EXPECT_EQ(stl.value()->bounds().max(), obj.value()->bounds().max());
}

TEST(Pipeline, LoadTransformRaycast) {
    auto loaded = gk::loadMesh(gk::test::dataFile("cube.obj"));
    ASSERT_TRUE(loaded.ok());
    std::shared_ptr<Mesh> mesh = std::move(loaded).value();

    // Centre the 2-unit cube on the origin, then lift it to z in [10, 12].
    mesh->transform(Transform::translation({-1, -1, -1}).then(Transform::translation({0, 0, 11})));
    const MeshRaycaster raycaster(mesh);

    const auto hit = raycaster.closestHit(Ray{{0.2, -0.4, 0}, {0, 0, 1}});
    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->t, 10.0, 1e-12);
    EXPECT_TRUE(gk::math::almostEqual(hit->normal, -Vec3::unitZ()));

    const auto through = raycaster.allHits(Ray{{0.2, -0.4, 0}, {0, 0, 1}});
    ASSERT_EQ(through.size(), 2U);
    EXPECT_NEAR(through[1].t - through[0].t, 2.0, 1e-12) << "cube thickness";
}

TEST(Pipeline, WorldSpaceAndLocalSpaceRaycastsAgree) {
    auto loaded = gk::loadMesh(gk::test::dataFile("cube_ascii.stl"));
    ASSERT_TRUE(loaded.ok());
    const auto local = std::shared_ptr<const Mesh>(std::move(loaded).value());

    const Transform placement =
        Transform::fromTRS({3, -2, 5}, gk::math::Quat::fromAxisAngle({1, 2, 0.5}, 0.8), {1.5, 1.5, 1.5});
    const auto world = std::make_shared<const Mesh>(local->transformed(placement));

    // Aim (with an unnormalised direction) at the placed cube's centre.
    const Vec3 origin{-20, 0, 0};
    const Ray worldRay{origin, placement.applyToPoint({1, 1, 1}) - origin};
    const auto worldHit = MeshRaycaster(world).closestHit(worldRay);
    // Same query in object space: transform the ray by the inverse placement.
    const auto localHit = MeshRaycaster(local).closestHit(worldRay.transformed(placement.inverse()));

    ASSERT_TRUE(worldHit && localHit);
    EXPECT_NEAR(worldHit->t, localHit->t, 1e-9) << "unnormalised ray directions preserve t";
    EXPECT_EQ(worldHit->faceIndex, localHit->faceIndex);
}

TEST(Pipeline, MetricsReflectWork) {
    auto& m = gk::metrics::MetricsRegistry::global();
    const auto loadsBefore = m.value("io.files_loaded");
    const auto callsBefore = m.value("io.load.calls");
    ASSERT_TRUE(gk::loadMesh(gk::test::dataFile("cube.obj")).ok());
    EXPECT_EQ(m.value("io.files_loaded"), loadsBefore + 1);
    EXPECT_EQ(m.value("io.load.calls"), callsBefore + 1);
}
