/// Example: build a mesh in code, then shoot a grid of rays at it using a
/// kernel chosen by *name* from the component registry, and print an ASCII
/// depth image plus the runtime counters.

#include <iostream>
#include <memory>
#include <string>

#include "geokernel/geokernel.hpp"

namespace {

/// Square pyramid: base [-1,1]^2 at z = 0, apex at (0,0,1.5). Outward CCW winding.
std::shared_ptr<gk::geometry::Mesh> makePyramid() {
    auto m = std::make_shared<gk::geometry::Mesh>();
    const auto a = m->addVertex({-1, -1, 0});
    const auto b = m->addVertex({1, -1, 0});
    const auto c = m->addVertex({1, 1, 0});
    const auto d = m->addVertex({-1, 1, 0});
    const auto apex = m->addVertex({0, 0, 1.5});
    m->addFace(a, c, b);  // base, facing -Z
    m->addFace(a, d, c);
    m->addFace(a, b, apex);
    m->addFace(b, c, apex);
    m->addFace(c, d, apex);
    m->addFace(d, a, apex);
    m->setName("pyramid");
    return m;
}

}  // namespace

int main() {
    auto& registry = gk::defaultRegistry();
    auto kernel = registry.createAs<gk::algorithms::IRayTriangleIntersector>(gk::components::kMollerTrumbore);
    if (!kernel) {
        std::cerr << "kernel not registered\n";
        return 1;
    }

    const gk::algorithms::MeshRaycaster raycaster(makePyramid(), kernel);
    const std::string shades = " .:-=+*#%@";  // far .. near

    constexpr int kSize = 24;
    for (int row = 0; row < kSize; ++row) {
        std::string line;
        for (int col = 0; col < kSize; ++col) {
            const double x = -1.2 + 2.4 * (col + 0.5) / kSize;
            const double y = 1.2 - 2.4 * (row + 0.5) / kSize;
            const auto hit = raycaster.closestHit(gk::geometry::Ray{{x, y, 5.0}, {0, 0, -1}},
                                                  gk::algorithms::CullMode::BackFaces);
            if (!hit) {
                line += "  ";
                continue;
            }
            const double height = hit->point.z / 1.5;  // 0 (base) .. 1 (apex)
            const auto idx = static_cast<std::size_t>(height * static_cast<double>(shades.size() - 1));
            line += std::string(2, shades[idx < shades.size() ? idx : shades.size() - 1]);
        }
        std::cout << line << '\n';
    }

    std::cout << "\nruntime counters:\n";
    for (const auto& [name, value] : gk::metrics::MetricsRegistry::global().snapshot()) {
        std::cout << "  " << name << " = " << value << '\n';
    }
    return 0;
}
