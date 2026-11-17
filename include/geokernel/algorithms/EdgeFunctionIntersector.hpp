#pragma once
/// @file EdgeFunctionIntersector.hpp
/// @brief Two-step intersector: ray/plane hit followed by three edge-function
/// inside tests. Slower than Möller–Trumbore but geometrically transparent;
/// used as an independent oracle to cross-validate the default kernel.

#include <string_view>

#include "geokernel/Export.hpp"
#include "geokernel/algorithms/IRayTriangleIntersector.hpp"
#include "geokernel/core/Component.hpp"

namespace gk::algorithms {

class GK_EXPORT EdgeFunctionIntersector final
    : public core::Component<EdgeFunctionIntersector, IRayTriangleIntersector> {
public:
    static constexpr std::string_view kComponentName = "gk.algorithms.EdgeFunction";

    [[nodiscard]] std::optional<TriangleHit> intersect(const geometry::Ray& ray,
                                                       const geometry::Triangle& tri,
                                                       CullMode cull) const noexcept override;
};

}  // namespace gk::algorithms
