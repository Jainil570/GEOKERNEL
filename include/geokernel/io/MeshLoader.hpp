#pragma once
/// @file MeshLoader.hpp
/// @brief Facade that picks a reader by file extension via the registry.
///
/// MeshLoader never names a concrete reader type: it asks the registry for
/// every component implementing IMeshReader and matches extensions. Adding a
/// format therefore requires only registering a new component.

#include <filesystem>
#include <istream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "geokernel/Export.hpp"
#include "geokernel/core/ComponentRegistry.hpp"
#include "geokernel/core/Result.hpp"
#include "geokernel/geometry/Mesh.hpp"
#include "geokernel/io/IMeshReader.hpp"

namespace gk::io {

class GK_EXPORT MeshLoader {
public:
    /// The registry must outlive the loader.
    explicit MeshLoader(const core::ComponentRegistry& registry) noexcept : registry_(&registry) {}

    /// Loads a file. Errors: NotFound, IoError, UnsupportedFormat, ParseError, LimitExceeded.
    [[nodiscard]] core::Result<std::unique_ptr<geometry::Mesh>> load(const std::filesystem::path& path,
                                                                     const ReadOptions& options = {}) const;

    /// Loads from a seekable binary stream; `extension` selects the reader (".stl").
    [[nodiscard]] core::Result<std::unique_ptr<geometry::Mesh>> load(std::istream& in,
                                                                     std::string_view extension,
                                                                     const ReadOptions& options = {}) const;

    /// Reader for an extension (case-insensitive, with or without the dot), or nullptr.
    [[nodiscard]] std::shared_ptr<IMeshReader> findReader(std::string_view extension) const;

    /// Sorted, de-duplicated list of every extension any registered reader handles.
    [[nodiscard]] std::vector<std::string> supportedExtensions() const;

private:
    const core::ComponentRegistry* registry_;
};

}  // namespace gk::io
