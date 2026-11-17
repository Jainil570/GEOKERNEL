#pragma once
/// @file IMeshReader.hpp
/// @brief Interface implemented by every mesh file-format reader.
///
/// New formats are added by implementing this interface and registering the
/// component — MeshLoader discovers readers through the registry, so no
/// existing code changes (open/closed principle; see examples/custom_reader).

#include <cstddef>
#include <istream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "geokernel/core/IBase.hpp"
#include "geokernel/core/Result.hpp"
#include "geokernel/geometry/Mesh.hpp"

namespace gk::io {

struct ReadOptions {
    /// Merge bit-identical vertices (STL stores 3 independent vertices per facet).
    bool weldVertices = true;
    /// Defensive limit against corrupt or hostile files (allocation bombs).
    std::size_t maxTriangles = 50'000'000;
    /// Run Mesh::validate() before returning.
    bool validate = true;
};

class IMeshReader : public virtual core::IBase {
    GK_DECLARE_INTERFACE("gk.io.IMeshReader")

public:
    /// Short human-readable format name, e.g. "STL".
    [[nodiscard]] virtual std::string_view formatName() const noexcept = 0;

    /// Lower-case extensions including the dot, e.g. {".stl"}.
    [[nodiscard]] virtual std::vector<std::string> fileExtensions() const = 0;

    /// Parses a mesh from a *seekable* stream opened in binary mode.
    /// Never throws for malformed input: problems are reported as Error.
    [[nodiscard]] virtual core::Result<std::unique_ptr<geometry::Mesh>> read(
        std::istream& in, const ReadOptions& options) const = 0;
};

}  // namespace gk::io
