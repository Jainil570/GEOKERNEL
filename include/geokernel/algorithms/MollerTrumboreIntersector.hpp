#pragma once
/// @file MollerTrumboreIntersector.hpp
/// @brief Möller & Trumbore (1997), "Fast, Minimum Storage Ray/Triangle
/// Intersection". Default kernel: no precomputed plane, ~1 division per test.

#include <string_view>

#include "geokernel/Export.hpp"
#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/core/Component.hpp"

namespace gk::algorithms {

class GK_EXPORT MollerTrumboreIntersector final
    : public core::Component<MollerTrumboreIntersector, IRayTriangleIntersector> {
public:
    static constexpr std::string_view kComponentName = "gk.algorithms.MollerTrumbore";

    [[nodiscard]] std::optional<TriangleHit> intersect(const geometry::Ray& ray,
                                                       const geometry::Triangle& tri,
                                                       CullMode cull) const noexcept override;
};

}  // namespace gk::algorithms
