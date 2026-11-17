#include "geokernel/io/ObjReader.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "core/LogInternal.hpp"
#include "geokernel/core/Metrics.hpp"
#include "io/detail/ParseUtils.hpp"

namespace gk::io {
namespace {

using core::ErrorCode;
using core::makeError;
using geometry::Mesh;
using math::Vec3;

constexpr std::array<std::string_view, 9> kIgnoredStatements{"vt", "vn", "vp",     "g",     "s",
                                                             "l",  "p",  "usemtl", "mtllib"};

bool isIgnored(std::string_view keyword) noexcept {
    for (const auto k : kIgnoredStatements) {
        if (keyword == k) {
            return true;
        }
    }
    return false;
}

core::Error lineError(std::size_t line, const std::string& what) {
    return makeError(ErrorCode::ParseError, "OBJ line " + std::to_string(line) + ": " + what);
}

}  // namespace

core::Result<std::unique_ptr<Mesh>> ObjReader::read(std::istream& in, const ReadOptions& options) const {
    if (!in) {
        return makeError(ErrorCode::IoError, "OBJ: input stream is not readable");
    }

    std::vector<Vec3> positions;
    std::vector<Mesh::Face> faces;
    std::string name;

    // Positive indices may legally reference vertices declared later, so the
    // range check runs after the whole file is read; remember where the
    // largest index occurred for a precise diagnostic.
    long long maxIndex = -1;
    std::size_t maxIndexLine = 0;
    std::uint64_t ignored = 0;

    std::string raw;
    std::vector<std::string_view> tokens;
    std::vector<long long> polygon;
    std::size_t lineNo = 0;

    while (std::getline(in, raw)) {
        ++lineNo;
        std::string_view line = raw;
        if (const auto hash = line.find('#'); hash != std::string_view::npos) {
            line = line.substr(0, hash);
        }
        detail::splitWhitespace(line, tokens);
        if (tokens.empty()) {
            continue;
        }
        const std::string_view keyword = tokens.front();

        if (keyword == "v") {
            if (tokens.size() < 4) {
                return lineError(lineNo, "vertex needs 3 coordinates");
            }
            std::array<double, 3> xyz{};
            for (std::size_t k = 0; k < 3; ++k) {
                if (!detail::parseDouble(tokens[k + 1], xyz[k])) {
                    return lineError(lineNo, "invalid number '" + std::string(tokens[k + 1]) + "'");
                }
            }
            positions.emplace_back(xyz[0], xyz[1], xyz[2]);  // optional w / RGB ignored
        } else if (keyword == "f") {
            if (tokens.size() < 4) {
                return lineError(lineNo, "face needs at least 3 vertices");
            }
            polygon.clear();
            for (std::size_t k = 1; k < tokens.size(); ++k) {
                const std::string_view ref = tokens[k].substr(0, tokens[k].find('/'));
                long long idx = 0;
                if (!detail::parseInt64(ref, idx) || idx == 0) {
                    return lineError(lineNo, "invalid vertex reference '" + std::string(tokens[k]) + "'");
                }
                if (idx < 0) {  // relative to the vertices declared so far
                    idx += static_cast<long long>(positions.size());
                    if (idx < 0) {
                        return lineError(lineNo, "relative index '" + std::string(ref) + "' out of range");
                    }
                } else {
                    idx -= 1;  // OBJ is 1-based
                }
                if (idx > maxIndex) {
                    maxIndex = idx;
                    maxIndexLine = lineNo;
                }
                polygon.push_back(idx);
            }
            // Fan triangulation (exact for convex polygons, the common case).
            for (std::size_t k = 1; k + 1 < polygon.size(); ++k) {
                if (faces.size() >= options.maxTriangles) {
                    return makeError(ErrorCode::LimitExceeded, "OBJ exceeds the triangle limit of " +
                                                                   std::to_string(options.maxTriangles));
                }
                // Narrowing is safe: maxIndex is range-checked against the
                // vertex count (which fits Mesh::Index) after the loop.
                faces.push_back({static_cast<Mesh::Index>(polygon[0]), static_cast<Mesh::Index>(polygon[k]),
                                 static_cast<Mesh::Index>(polygon[k + 1])});
            }
        } else if (keyword == "o") {
            if (name.empty() && tokens.size() > 1) {
                name = std::string(detail::trim(line.substr(line.find('o') + 1)));
            }
        } else if (isIgnored(keyword)) {
            ++ignored;
        } else {
            ++ignored;
            GK_LOG_DEBUG("OBJ line {}: unsupported statement '{}' ignored", lineNo, keyword);
        }
    }

    if (in.bad()) {
        return makeError(ErrorCode::IoError, "OBJ: stream error after line " + std::to_string(lineNo));
    }
    if (positions.size() > std::numeric_limits<Mesh::Index>::max()) {
        return makeError(ErrorCode::LimitExceeded, "OBJ: vertex count exceeds 32-bit index range");
    }
    if (maxIndex >= static_cast<long long>(positions.size())) {
        return lineError(maxIndexLine, "vertex index " + std::to_string(maxIndex + 1) + " out of range (" +
                                           std::to_string(positions.size()) + " vertices declared)");
    }

    auto mesh = std::make_unique<Mesh>(std::move(positions), std::move(faces), std::move(name));
    if (options.validate) {
        GK_RETURN_IF_ERROR(mesh->validate());
    }

    auto& m = metrics::MetricsRegistry::global();
    m.increment("io.obj.files");
    m.increment("io.obj.triangles", mesh->faceCount());
    m.increment("io.obj.ignored_statements", ignored);
    GK_LOG_DEBUG("OBJ: read {} faces / {} vertices ({} statements ignored)", mesh->faceCount(),
                 mesh->vertexCount(), ignored);
    return mesh;
}

}  // namespace gk::io
