#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <string>

#include "TestHelpers.hpp"
#include "geokernel/algorithms/EdgeFunctionIntersector.hpp"
#include "geokernel/algorithms/MollerTrumboreIntersector.hpp"

using gk::algorithms::CullMode;
using gk::algorithms::EdgeFunctionIntersector;
using gk::algorithms::IRayTriangleIntersector;
using gk::algorithms::MollerTrumboreIntersector;
using gk::geometry::Ray;
using gk::geometry::Triangle;
using gk::math::Vec3;

/// Every behavioural test runs against BOTH kernels: the interface contract,
/// not one implementation, is what is being tested.
class RayTriangleContract : public ::testing::TestWithParam<std::string> {
protected:
    void SetUp() override {
        if (GetParam() == "MollerTrumbore") {
            kernel_ = std::make_shared<MollerTrumboreIntersector>();
        } else {
            kernel_ = std::make_shared<EdgeFunctionIntersector>();
        }
    }
    const IRayTriangleIntersector& kernel() const { return *kernel_; }

private:
    std::shared_ptr<IRayTriangleIntersector> kernel_;
};

INSTANTIATE_TEST_SUITE_P(Kernels, RayTriangleContract, ::testing::Values("MollerTrumbore", "EdgeFunction"),
                         [](const auto& info) { return info.param; });

TEST_P(RayTriangleContract, HitsInteriorWithCorrectBarycentrics) {
    const Triangle tri = gk::test::makeXYTriangle();
    const auto hit = kernel().intersect(Ray{{0.25, 0.25, 1.0}, {0, 0, -1}}, tri);
    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->t, 1.0, 1e-12);
    EXPECT_NEAR(hit->u, 0.25, 1e-12);
    EXPECT_NEAR(hit->v, 0.25, 1e-12);
    EXPECT_TRUE(hit->frontFace);
    EXPECT_TRUE(gk::math::almostEqual(tri.pointAt(hit->u, hit->v), Vec3{0.25, 0.25, 0}));
}

TEST_P(RayTriangleContract, MissesOutsideTriangle) {
    const Triangle tri = gk::test::makeXYTriangle();
    EXPECT_FALSE(kernel().intersect(Ray{{0.8, 0.8, 1.0}, {0, 0, -1}}, tri).has_value());
    EXPECT_FALSE(kernel().intersect(Ray{{-0.1, 0.5, 1.0}, {0, 0, -1}}, tri).has_value());
}

TEST_P(RayTriangleContract, ParallelRayNeverHits) {
    const Triangle tri = gk::test::makeXYTriangle();
    EXPECT_FALSE(kernel().intersect(Ray{{-1, 0.2, 0}, {1, 0, 0}}, tri).has_value()) << "in-plane";
    EXPECT_FALSE(kernel().intersect(Ray{{-1, 0.2, 1}, {1, 0, 0}}, tri).has_value()) << "above plane";
}

TEST_P(RayTriangleContract, RespectsRayInterval) {
    const Triangle tri = gk::test::makeXYTriangle();
    EXPECT_FALSE(kernel().intersect(Ray{{0.2, 0.2, -1}, {0, 0, -1}}, tri).has_value()) << "behind origin";
    EXPECT_FALSE(kernel().intersect(Ray{{0.2, 0.2, 2}, {0, 0, -1}, 0.0, 1.5}, tri).has_value())
        << "beyond tMax";
    EXPECT_TRUE(kernel().intersect(Ray{{0.2, 0.2, 2}, {0, 0, -1}, 0.0, 2.5}, tri).has_value());
}

TEST_P(RayTriangleContract, BackFaceCulling) {
    const Triangle tri = gk::test::makeXYTriangle();
    const Ray fromBelow{{0.2, 0.2, -1}, {0, 0, 1}};
    const auto hit = kernel().intersect(fromBelow, tri, CullMode::None);
    ASSERT_TRUE(hit.has_value());
    EXPECT_FALSE(hit->frontFace);
    EXPECT_FALSE(kernel().intersect(fromBelow, tri, CullMode::BackFaces).has_value());
    EXPECT_TRUE(kernel().intersect(Ray{{0.2, 0.2, 1}, {0, 0, -1}}, tri, CullMode::BackFaces).has_value());
}

TEST_P(RayTriangleContract, DegenerateTriangleNeverHits) {
    const Triangle line{{0, 0, 0}, {1, 1, 0}, {2, 2, 0}};
    EXPECT_FALSE(kernel().intersect(Ray{{1, 1, 1}, {0, 0, -1}}, line).has_value());
}

TEST_P(RayTriangleContract, UnnormalisedDirectionScalesT) {
    const auto hit = kernel().intersect(Ray{{0.2, 0.2, 4}, {0, 0, -2}}, gk::test::makeXYTriangle());
    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->t, 2.0, 1e-12) << "t is measured in units of |direction|";
}

/// Differential test: two independent algorithms must agree on random input.
TEST(RayTriangleCrossCheck, KernelsAgreeOnRandomRays) {
    MollerTrumboreIntersector mt;
    EdgeFunctionIntersector ef;
    std::mt19937 rng(20261117);  // fixed seed => reproducible
    std::uniform_real_distribution<double> coord(-2.0, 2.0);
    const auto rnd = [&] { return Vec3{coord(rng), coord(rng), coord(rng)}; };

    int hits = 0;
    for (int i = 0; i < 5000; ++i) {
        const Triangle tri{rnd(), rnd(), rnd()};
        const Ray ray{rnd() * 3.0, rnd()};
        const auto a = mt.intersect(ray, tri, CullMode::None);
        const auto b = ef.intersect(ray, tri, CullMode::None);
        if (a.has_value() != b.has_value()) {
            // Only acceptable on an edge, where round-off decides; those are
            // astronomically rare with random doubles — flag them loudly.
            ADD_FAILURE() << "disagreement at iteration " << i;
            continue;
        }
        if (a) {
            ++hits;
            EXPECT_NEAR(a->t, b->t, 1e-9 * std::max(1.0, std::abs(a->t)));
            EXPECT_NEAR(a->u, b->u, 1e-9);
            EXPECT_NEAR(a->v, b->v, 1e-9);
            EXPECT_EQ(a->frontFace, b->frontFace);
        }
    }
    EXPECT_GT(hits, 100) << "sample should exercise the hit path";
}

TEST(RayTriangle, ComponentsReportRegistryNames) {
    EXPECT_EQ(MollerTrumboreIntersector{}.componentName(), "gk.algorithms.MollerTrumbore");
    EXPECT_EQ(EdgeFunctionIntersector{}.componentName(), "gk.algorithms.EdgeFunction");
}
