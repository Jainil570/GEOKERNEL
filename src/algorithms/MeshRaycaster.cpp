#include "geokernel/algorithms/MeshRaycaster.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "core/LogInternal.hpp"
#include "geokernel/algorithms/MollerTrumboreIntersector.hpp"
#include "geokernel/core/Metrics.hpp"
#include "geokernel/math/Tolerance.hpp"

namespace gk::algorithms {

namespace {

/// Counter handles are resolved once; increments afterwards are lock-free.
struct RaycastCounters {
    metrics::Counter& calls;
    metrics::Counter& invalidRays;
    metrics::Counter& boundsRejects;
    metrics::Counter& triangleTests;
    metrics::Counter& hits;

    static RaycastCounters& get() {
        auto& m = metrics::MetricsRegistry::global();
        static RaycastCounters counters{m.counter("raycast.calls"), m.counter("raycast.invalid_rays"),
                                        m.counter("raycast.bounds_rejects"),
                                        m.counter("raycast.triangle_tests"), m.counter("raycast.hits")};
        return counters;
    }
};

}  // namespace

MeshRaycaster::MeshRaycaster(std::shared_ptr<const geometry::Mesh> mesh)
    : MeshRaycaster(std::move(mesh), std::make_shared<MollerTrumboreIntersector>()) {}

MeshRaycaster::MeshRaycaster(std::shared_ptr<const geometry::Mesh> mesh,
                             std::shared_ptr<const IRayTriangleIntersector> intersector)
    : mesh_(std::move(mesh)), intersector_(std::move(intersector)) {
    if (!mesh_) {
        throw std::invalid_argument("MeshRaycaster: mesh must not be null");
    }
    if (!intersector_) {
        throw std::invalid_argument("MeshRaycaster: intersector must not be null");
    }
    GK_LOG_DEBUG("MeshRaycaster: {} faces, kernel '{}'", mesh_->faceCount(), intersector_->componentName());
}

std::optional<MeshHit> MeshRaycaster::closestHit(const geometry::Ray& ray, CullMode cull) const {
    auto hits = trace(ray, cull, Mode::Closest);
    if (hits.empty()) {
        return std::nullopt;
    }
    return hits.front();
}

bool MeshRaycaster::anyHit(const geometry::Ray& ray, CullMode cull) const {
    return !trace(ray, cull, Mode::Any).empty();
}

std::vector<MeshHit> MeshRaycaster::allHits(const geometry::Ray& ray, CullMode cull) const {
    return trace(ray, cull, Mode::All);
}

std::vector<MeshHit> MeshRaycaster::trace(const geometry::Ray& ray, CullMode cull, Mode mode) const {
    auto& counters = RaycastCounters::get();
    counters.calls.increment();

    std::vector<MeshHit> hits;
    if (!ray.isValid()) {
        counters.invalidRays.increment();
        GK_LOG_WARN("MeshRaycaster: invalid ray rejected");
        return hits;
    }

    // Early-out + interval clipping against the mesh bounds.
    const auto span = mesh_->bounds().intersect(ray);
    if (!span) {
        counters.boundsRejects.increment();
        return hits;
    }
    // Pad the slab interval so a triangle lying exactly on a box face is not
    // lost to round-off between the slab test and the triangle kernel.
    const double pad = math::kLinearTolerance * std::max(1.0, std::abs(span->second));
    geometry::Ray clipped = ray;
    clipped.tMin = std::max(ray.tMin, span->first - pad);
    clipped.tMax = std::min(ray.tMax, span->second + pad);

    const auto& vertices = mesh_->vertices();
    const auto& faces = mesh_->faces();
    std::uint64_t tests = 0;

    for (std::size_t i = 0; i < faces.size(); ++i) {
        const auto& f = faces[i];
        const geometry::Triangle tri{vertices[f[0]], vertices[f[1]], vertices[f[2]]};
        ++tests;
        const auto hit = intersector_->intersect(clipped, tri, cull);
        if (!hit) {
            continue;
        }

        MeshHit mh{hit->t, i, hit->u, hit->v, ray.at(hit->t), tri.normal(), hit->frontFace};
        if (mode == Mode::Closest) {
            clipped.tMax = hit->t;  // shrink the interval: later hits must be nearer
            hits.assign(1, mh);
        } else {
            hits.push_back(mh);
            if (mode == Mode::Any) {
                break;
            }
        }
    }

    if (mode == Mode::All) {
        std::sort(hits.begin(), hits.end(), [](const MeshHit& a, const MeshHit& b) { return a.t < b.t; });
    }

    counters.triangleTests.increment(tests);
    counters.hits.increment(hits.size());
    return hits;
}

}  // namespace gk::algorithms
