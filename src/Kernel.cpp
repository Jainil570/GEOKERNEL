#include "geokernel/Kernel.hpp"

#include <array>

#include "core/LogInternal.hpp"
#include "geokernel/Version.hpp"
#include "geokernel/algorithms/EdgeFunctionIntersector.hpp"
#include "geokernel/algorithms/MollerTrumboreIntersector.hpp"
#include "geokernel/io/MeshLoader.hpp"
#include "geokernel/io/ObjReader.hpp"
#include "geokernel/io/StlReader.hpp"

namespace gk {

core::Status registerBuiltinComponents(core::ComponentRegistry& registry) {
    const std::array<core::Status, 4> results{
        registry.registerComponent<io::StlReader>("STL reader (binary + ASCII)", kVersion.string),
        registry.registerComponent<io::ObjReader>("Wavefront OBJ reader (geometry subset)", kVersion.string),
        registry.registerComponent<algorithms::MollerTrumboreIntersector>(
            "Moller-Trumbore ray/triangle intersection", kVersion.string),
        registry.registerComponent<algorithms::EdgeFunctionIntersector>(
            "Plane + edge-function ray/triangle intersection", kVersion.string),
    };
    for (const auto& status : results) {
        if (!status.ok()) {
            return status;
        }
    }
    return core::Status::success();
}

core::ComponentRegistry& defaultRegistry() {
    static core::ComponentRegistry registry;
    // Magic-static initialisation runs exactly once, even under concurrent first calls.
    static const bool initialised = [] {
        if (const auto status = registerBuiltinComponents(registry); !status.ok()) {
            GK_LOG_CRITICAL("failed to register built-in components: {}", status.error().toString());
            return false;
        }
        GK_LOG_DEBUG("GeoKernel {} default registry ready ({} components)", kVersion.string, registry.size());
        return true;
    }();
    static_cast<void>(initialised);
    return registry;
}

core::Result<std::unique_ptr<geometry::Mesh>> loadMesh(const std::filesystem::path& path,
                                                       const io::ReadOptions& options) {
    return io::MeshLoader(defaultRegistry()).load(path, options);
}

}  // namespace gk
