#include "geokernel/io/MeshLoader.hpp"

#include <algorithm>
#include <fstream>
#include <system_error>

#include "core/LogInternal.hpp"
#include "geokernel/core/Metrics.hpp"
#include "io/detail/ParseUtils.hpp"

namespace gk::io {
namespace {

using core::ErrorCode;
using core::makeError;

std::string normalizeExtension(std::string_view extension) {
    std::string ext = detail::toLower(detail::trim(extension));
    if (!ext.empty() && ext.front() != '.') {
        ext.insert(ext.begin(), '.');
    }
    return ext;
}

std::string joinExtensions(const std::vector<std::string>& exts) {
    std::string out;
    for (const auto& e : exts) {
        out += (out.empty() ? "" : ", ") + e;
    }
    return out.empty() ? "none" : out;
}

}  // namespace

std::shared_ptr<IMeshReader> MeshLoader::findReader(std::string_view extension) const {
    const std::string ext = normalizeExtension(extension);
    if (ext.empty()) {
        return nullptr;
    }
    // Registry names are sorted, so resolution is deterministic when two
    // readers claim the same extension (the alphabetically first wins).
    for (const auto& name : registry_->findImplementing(IMeshReader::iid())) {
        auto reader = registry_->createAs<IMeshReader>(name);
        if (!reader) {
            continue;
        }
        const auto exts = reader->fileExtensions();
        if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {
            return reader;
        }
    }
    return nullptr;
}

std::vector<std::string> MeshLoader::supportedExtensions() const {
    std::vector<std::string> out;
    for (const auto& reader : registry_->createAllImplementing<IMeshReader>()) {
        for (const auto& e : reader->fileExtensions()) {
            out.push_back(normalizeExtension(e));
        }
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

core::Result<std::unique_ptr<geometry::Mesh>> MeshLoader::load(std::istream& in, std::string_view extension,
                                                               const ReadOptions& options) const {
    auto reader = findReader(extension);
    if (!reader) {
        return makeError(ErrorCode::UnsupportedFormat,
                         "no reader registered for '" + std::string(extension) +
                             "' (supported: " + joinExtensions(supportedExtensions()) + ")");
    }
    return reader->read(in, options);
}

core::Result<std::unique_ptr<geometry::Mesh>> MeshLoader::load(const std::filesystem::path& path,
                                                               const ReadOptions& options) const {
    metrics::ScopedTimer timer("io.load");
    auto& m = metrics::MetricsRegistry::global();
    const std::string display = path.u8string();

    const auto fail = [&](core::Error error) -> core::Result<std::unique_ptr<geometry::Mesh>> {
        m.increment("io.load_failures");
        GK_LOG_ERROR("load '{}' failed: {}", display, error.toString());
        return error;
    };

    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return fail(makeError(ErrorCode::NotFound, "file not found: " + display));
    }
    if (std::filesystem::is_directory(path, ec)) {
        return fail(makeError(ErrorCode::InvalidArgument, "path is a directory: " + display));
    }

    const std::string ext = path.extension().u8string();
    if (!findReader(ext)) {
        return fail(makeError(ErrorCode::UnsupportedFormat,
                              "unsupported extension '" + ext +
                                  "' (supported: " + joinExtensions(supportedExtensions()) + ")"));
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return fail(makeError(ErrorCode::IoError, "cannot open file: " + display));
    }

    auto result = load(file, ext, options);
    if (!result) {
        return fail(result.error());
    }

    auto& mesh = result.value();
    if (mesh->name().empty()) {
        mesh->setName(path.stem().u8string());
    }
    m.increment("io.files_loaded");
    GK_LOG_INFO("loaded '{}': {} faces, {} vertices in {} us", display, mesh->faceCount(),
                mesh->vertexCount(), timer.elapsed().count());
    return result;
}

}  // namespace gk::io
