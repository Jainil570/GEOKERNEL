#pragma once
/// @file Kernel.hpp
/// @brief Composition root: wires the built-in components into a registry.
///
/// This is the only translation unit that names every concrete component.
/// Registration is explicit (not via static self-registering objects) because
/// static libraries silently drop unreferenced object files and static
/// initialisation order across TUs is unspecified. See docs/adr/0002.

#include <filesystem>
#include <memory>

#include "geokernel/Export.hpp"
#include "geokernel/core/ComponentRegistry.hpp"
#include "geokernel/core/Result.hpp"
#include "geokernel/geometry/Mesh.hpp"
#include "geokernel/io/IMeshReader.hpp"

namespace gk {

/// Component names of the built-ins (stable public identifiers).
namespace components {
inline constexpr const char* kStlReader = "gk.io.StlReader";
inline constexpr const char* kObjReader = "gk.io.ObjReader";
inline constexpr const char* kMollerTrumbore = "gk.algorithms.MollerTrumbore";
inline constexpr const char* kEdgeFunction = "gk.algorithms.EdgeFunction";
}  // namespace components

/// Registers every built-in component into `registry`.
/// Fails with AlreadyExists if any of them is already present.
GK_EXPORT core::Status registerBuiltinComponents(core::ComponentRegistry& registry);

/// Process-wide registry, populated with the built-ins on first use (thread-safe).
/// Applications may register additional plugins into it.
[[nodiscard]] GK_EXPORT core::ComponentRegistry& defaultRegistry();

/// Convenience: load a mesh file through the default registry.
[[nodiscard]] GK_EXPORT core::Result<std::unique_ptr<geometry::Mesh>> loadMesh(
    const std::filesystem::path& path, const io::ReadOptions& options = {});

}  // namespace gk
