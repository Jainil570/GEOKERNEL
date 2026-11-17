#pragma once
/// @file StlReader.hpp
/// @brief STL reader (binary and ASCII, auto-detected).
///
/// Detection does not trust the "solid" prefix (many binary exporters write
/// it into the 80-byte header). A stream is binary when its size equals
/// 84 + 50 * triangleCount; otherwise a leading "solid" selects ASCII.
/// Facet normals in the file are ignored and recomputed from winding.

#include <string_view>

#include "geokernel/Export.hpp"
#include "geokernel/core/Component.hpp"
#include "geokernel/io/IMeshReader.hpp"

namespace gk::io {

class GK_EXPORT StlReader final : public core::Component<StlReader, IMeshReader> {
public:
    static constexpr std::string_view kComponentName = "gk.io.StlReader";

    [[nodiscard]] std::string_view formatName() const noexcept override { return "STL"; }
    [[nodiscard]] std::vector<std::string> fileExtensions() const override { return {".stl"}; }
    [[nodiscard]] core::Result<std::unique_ptr<geometry::Mesh>> read(
        std::istream& in, const ReadOptions& options) const override;
};

}  // namespace gk::io
