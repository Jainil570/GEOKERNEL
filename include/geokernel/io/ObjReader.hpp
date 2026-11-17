#pragma once
/// @file ObjReader.hpp
/// @brief Wavefront OBJ reader (geometry subset).
///
/// Supported: `v`, `f` with `i`, `i/t`, `i//n`, `i/t/n` references, negative
/// (relative) indices, n-gons (fan-triangulated), `o` (mesh name), comments.
/// Ignored without error: `vt`, `vn`, `vp`, `g`, `s`, `l`, `p`, `usemtl`, `mtllib`.

#include <string_view>

#include "geokernel/Export.hpp"
#include "geokernel/core/Component.hpp"
#include "geokernel/io/IMeshReader.hpp"

namespace gk::io {

class GK_EXPORT ObjReader final : public core::Component<ObjReader, IMeshReader> {
public:
    static constexpr std::string_view kComponentName = "gk.io.ObjReader";

    [[nodiscard]] std::string_view formatName() const noexcept override { return "OBJ"; }
    [[nodiscard]] std::vector<std::string> fileExtensions() const override { return {".obj"}; }
    [[nodiscard]] core::Result<std::unique_ptr<geometry::Mesh>> read(
        std::istream& in, const ReadOptions& options) const override;
};

}  // namespace gk::io
