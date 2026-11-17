/// Micro-benchmarks for the ray-intersection hot path.
/// Run: build the `release` preset, then  ./build/release/benchmarks/geokernel_bench

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <vector>

#include "geokernel/algorithms/EdgeFunctionIntersector.hpp"
#include "geokernel/algorithms/MeshRaycaster.hpp"
#include "geokernel/algorithms/MollerTrumboreIntersector.hpp"
#include "geokernel/core/Log.hpp"

namespace {

using gk::geometry::Mesh;
using gk::geometry::Ray;
using gk::geometry::Triangle;
using gk::math::Vec3;

struct Workload {
    std::vector<Triangle> triangles;
    std::vector<Ray> rays;
};

const Workload& randomWorkload() {
    static const Workload w = [] {
        Workload out;
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> u(-1.0, 1.0);
        const auto rnd = [&] { return Vec3{u(rng), u(rng), u(rng)}; };
        for (int i = 0; i < 4096; ++i) {
            out.triangles.push_back({rnd(), rnd(), rnd()});
            out.rays.emplace_back(rnd() * 3.0, rnd());
        }
        return out;
    }();
    return w;
}

/// UV sphere with `stacks * slices * 2` triangles.
std::shared_ptr<const Mesh> makeSphere(int stacks, int slices) {
    auto mesh = std::make_shared<Mesh>();
    const double pi = gk::math::kPi;
    for (int i = 0; i <= stacks; ++i) {
        const double phi = pi * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            const double theta = 2.0 * pi * j / slices;
            mesh->addVertex(
                {std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi)});
        }
    }
    const auto idx = [slices](int i, int j) { return static_cast<Mesh::Index>(i * (slices + 1) + j); };
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            mesh->addFace(idx(i, j), idx(i + 1, j), idx(i + 1, j + 1));
            mesh->addFace(idx(i, j), idx(i + 1, j + 1), idx(i, j + 1));
        }
    }
    return mesh;
}

template <typename Kernel>
void BM_RayTriangle(benchmark::State& state) {
    gk::log::setLevel(gk::log::Level::Off);
    const Kernel kernel;
    const auto& w = randomWorkload();
    std::size_t i = 0;
    std::size_t hits = 0;
    for (auto _ : state) {
        const auto hit =
            kernel.intersect(w.rays[i & 4095U], w.triangles[i & 4095U], gk::algorithms::CullMode::None);
        hits += hit.has_value() ? 1U : 0U;
        benchmark::DoNotOptimize(hit);
        ++i;
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
    state.counters["hit_rate"] = static_cast<double>(hits) / static_cast<double>(std::max<std::size_t>(i, 1));
}
BENCHMARK_TEMPLATE(BM_RayTriangle, gk::algorithms::MollerTrumboreIntersector);
BENCHMARK_TEMPLATE(BM_RayTriangle, gk::algorithms::EdgeFunctionIntersector);

void BM_MeshClosestHit(benchmark::State& state) {
    gk::log::setLevel(gk::log::Level::Off);
    const auto stacks = static_cast<int>(state.range(0));
    const gk::algorithms::MeshRaycaster raycaster(makeSphere(stacks, stacks * 2));
    const Ray ray{{0.123, -0.0456, 5.0}, {0, 0, -1}};
    for (auto _ : state) {
        benchmark::DoNotOptimize(raycaster.closestHit(ray));
    }
    state.counters["triangles"] = static_cast<double>(raycaster.mesh().faceCount());
}
BENCHMARK(BM_MeshClosestHit)->Arg(8)->Arg(32)->Arg(128);

void BM_MeshBoundsReject(benchmark::State& state) {
    gk::log::setLevel(gk::log::Level::Off);
    const gk::algorithms::MeshRaycaster raycaster(makeSphere(128, 256));
    const Ray miss{{5, 5, 5}, {0, 0, 1}};
    for (auto _ : state) {
        benchmark::DoNotOptimize(raycaster.closestHit(miss));
    }
}
BENCHMARK(BM_MeshBoundsReject);

}  // namespace
