#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <stdexcept>

#include "TestHelpers.hpp"
#include "geokernel/Kernel.hpp"
#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/algorithms/MeshRaycaster.hpp"
#include "geokernel/core/Metrics.hpp"

using gk::algorithms::CullMode;
using gk::algorithms::IRayTriangleIntersector;
using gk::algorithms::MeshRaycaster;
using gk::geometry::Mesh;
using gk::geometry::Ray;
using gk::math::Vec3;

namespace {
std::shared_ptr<const Mesh> cube() {
    return std::make_shared<Mesh>(gk::test::makeUnitCube());
}
}  // namespace

TEST(MeshRaycaster, ClosestHitOnCubeFace) {
    const MeshRaycaster rc(cube());
    // Rays deliberately avoid the cube's diagonal edges (x == y etc.), where a
    // non-watertight kernel may legitimately report either adjacent triangle.
    const auto hit = rc.closestHit(Ray{{0.25, 0.6, 5}, {0, 0, -1}});
    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->t, 4.0, 1e-12);
    EXPECT_TRUE(gk::math::almostEqual(hit->point, Vec3{0.25, 0.6, 1}));
    EXPECT_TRUE(gk::math::almostEqual(hit->normal, Vec3::unitZ()));
    EXPECT_TRUE(hit->frontFace);
    EXPECT_LT(hit->faceIndex, 12U);
}

TEST(MeshRaycaster, AllHitsSortedEntryThenExit) {
    const MeshRaycaster rc(cube());
    const auto hits = rc.allHits(Ray{{0.3, 0.6, -2}, {0, 0, 1}});
    ASSERT_EQ(hits.size(), 2U);
    EXPECT_NEAR(hits[0].t, 2.0, 1e-12);
    EXPECT_NEAR(hits[1].t, 3.0, 1e-12);
    EXPECT_TRUE(hits[0].frontFace);
    EXPECT_FALSE(hits[1].frontFace);
}

TEST(MeshRaycaster, BackFaceCullingFromInside) {
    const MeshRaycaster rc(cube());
    const Ray fromCentre{{0.5, 0.3, 0.6}, {1, 0, 0}};
    EXPECT_TRUE(rc.closestHit(fromCentre, CullMode::None).has_value());
    EXPECT_FALSE(rc.closestHit(fromCentre, CullMode::BackFaces).has_value());
}

TEST(MeshRaycaster, MissesAndBoundsRejection) {
    const MeshRaycaster rc(cube());
    auto& m = gk::metrics::MetricsRegistry::global();
    const auto before = m.value("raycast.bounds_rejects");
    EXPECT_FALSE(rc.closestHit(Ray{{5, 5, 5}, {0, 0, 1}}).has_value());
    EXPECT_FALSE(rc.anyHit(Ray{{5, 5, 5}, {0, 0, 1}}));
    EXPECT_EQ(m.value("raycast.bounds_rejects"), before + 2);
}

TEST(MeshRaycaster, AnyHitAndShortSegments) {
    const MeshRaycaster rc(cube());
    EXPECT_TRUE(rc.anyHit(Ray::fromPoints({0.3, 0.6, -1}, {0.3, 0.6, 2})));
    EXPECT_FALSE(rc.anyHit(Ray::fromPoints({0.3, 0.6, -1}, {0.3, 0.6, -0.5}))) << "segment stops short";
}

TEST(MeshRaycaster, InvalidRayIsRejectedNotCrashing) {
    const MeshRaycaster rc(cube());
    EXPECT_FALSE(rc.closestHit(Ray{{0, 0, 0}, {0, 0, 0}}).has_value());
    const double nan = std::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(rc.closestHit(Ray{{nan, 0, 0}, {0, 0, 1}}).has_value());
}

TEST(MeshRaycaster, NullArgumentsThrow) {
    EXPECT_THROW(MeshRaycaster(nullptr), std::invalid_argument);
    EXPECT_THROW(MeshRaycaster(cube(), nullptr), std::invalid_argument);
}

TEST(MeshRaycaster, KernelSelectedFromRegistryGivesSameAnswer) {
    auto& reg = gk::defaultRegistry();
    const auto ef = reg.createAs<IRayTriangleIntersector>(gk::components::kEdgeFunction);
    ASSERT_NE(ef, nullptr);

    const MeshRaycaster viaRegistry(cube(), ef);
    const MeshRaycaster viaDefault(cube());
    EXPECT_EQ(viaRegistry.intersector().componentName(), "gk.algorithms.EdgeFunction");

    const Ray ray{{-1, 0.25, 0.75}, Vec3{1, 0.1, -0.05}};
    const auto a = viaRegistry.closestHit(ray);
    const auto b = viaDefault.closestHit(ray);
    ASSERT_TRUE(a && b);
    EXPECT_NEAR(a->t, b->t, 1e-12);
    EXPECT_EQ(a->faceIndex, b->faceIndex);
}

TEST(MeshRaycaster, CountsTriangleTests) {
    const MeshRaycaster rc(cube());
    auto& m = gk::metrics::MetricsRegistry::global();
    const auto before = m.value("raycast.triangle_tests");
    static_cast<void>(rc.allHits(Ray{{0.5, 0.5, -1}, {0, 0, 1}}));
    EXPECT_EQ(m.value("raycast.triangle_tests"), before + 12) << "brute force tests every face once";
}
