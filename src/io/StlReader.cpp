#include "geokernel/io/StlReader.hpp"

#include <array>
#include <cctype>
#include <cstdint>
#include <string>

#include "core/LogInternal.hpp"
#include "geokernel/core/Metrics.hpp"
#include "io/detail/ParseUtils.hpp"
#include "io/detail/VertexWelder.hpp"

namespace gk::io {
namespace {

using core::ErrorCode;
using core::makeError;
using core::Result;
using core::Status;
using geometry::Mesh;
using math::Vec3;

constexpr std::uint64_t kHeaderBytes = 80;
constexpr std::uint64_t kPreambleBytes = kHeaderBytes + 4;  // header + uint32 count
constexpr std::uint64_t kFacetBytes = 50;                   // 12 floats + uint16 attribute

enum class StlKind { Binary, Ascii, Invalid };

// ---------------------------------------------------------------------------
//  Format detection
// ---------------------------------------------------------------------------
StlKind detect(std::istream& in, std::uint64_t size) {
    const auto start = in.tellg();

    if (size >= kPreambleBytes) {
        std::array<unsigned char, kPreambleBytes> preamble{};
        in.read(
            reinterpret_cast<char*>(preamble.data()),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            static_cast<std::streamsize>(preamble.size()));
        in.seekg(start);
        const std::uint64_t count = detail::readU32LE(preamble.data() + kHeaderBytes);
        if (kPreambleBytes + kFacetBytes * count == size) {
            return StlKind::Binary;  // exact size match is decisive
        }
    }

    // Skip leading whitespace, then look for the ASCII keyword.
    std::string head;
    char c = 0;
    while (head.size() < 5 && in.get(c)) {
        if (head.empty() && std::isspace(static_cast<unsigned char>(c)) != 0) {
            continue;
        }
        head.push_back(c);
    }
    in.clear();
    in.seekg(start);

    if (detail::iequals(head, "solid")) {
        return StlKind::Ascii;
    }
    return size >= kPreambleBytes ? StlKind::Binary : StlKind::Invalid;
}

// ---------------------------------------------------------------------------
//  Binary
// ---------------------------------------------------------------------------
Result<std::unique_ptr<Mesh>> readBinary(std::istream& in, std::uint64_t size, const ReadOptions& opt) {
    std::array<unsigned char, kPreambleBytes> preamble{};
    in.read(reinterpret_cast<char*>(preamble.data()),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            static_cast<std::streamsize>(preamble.size()));
    if (!in) {
        return makeError(ErrorCode::ParseError, "binary STL: truncated 84-byte preamble");
    }

    const std::uint64_t count = detail::readU32LE(preamble.data() + kHeaderBytes);
    if (count > opt.maxTriangles) {
        return makeError(ErrorCode::LimitExceeded, "binary STL declares " + std::to_string(count) +
                                                       " triangles (limit " +
                                                       std::to_string(opt.maxTriangles) + ")");
    }
    const std::uint64_t expected = kPreambleBytes + kFacetBytes * count;
    if (size < expected) {
        // Checked BEFORE allocating: a corrupt count cannot trigger a huge reserve().
        return makeError(ErrorCode::ParseError, "binary STL truncated: header declares " +
                                                    std::to_string(count) + " triangles (" +
                                                    std::to_string(expected) + " bytes) but stream has " +
                                                    std::to_string(size) + " bytes");
    }
    if (size > expected) {
        GK_LOG_WARN("binary STL: ignoring {} trailing byte(s)", size - expected);
    }

    auto mesh = std::make_unique<Mesh>();
    const auto n = static_cast<std::size_t>(count);
    mesh->reserve(opt.weldVertices ? n / 2 + 3 : n * 3, n);
    detail::VertexWelder welder(*mesh, opt.weldVertices);
    welder.reserve(n / 2 + 3);

    std::array<unsigned char, kFacetBytes> rec{};
    for (std::size_t i = 0; i < n; ++i) {
        in.read(reinterpret_cast<char*>(rec.data()),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                static_cast<std::streamsize>(rec.size()));
        if (!in) {
            return makeError(ErrorCode::ParseError,
                             "binary STL: read failed at triangle " + std::to_string(i));
        }
        std::array<Mesh::Index, 3> idx{};
        for (std::size_t k = 0; k < 3; ++k) {
            const unsigned char* base = rec.data() + 12 + 12 * k;  // skip the 12-byte normal
            const Vec3 p{static_cast<double>(detail::readF32LE(base)),
                         static_cast<double>(detail::readF32LE(base + 4)),
                         static_cast<double>(detail::readF32LE(base + 8))};
            if (!p.isFinite()) {
                return makeError(ErrorCode::ParseError,
                                 "binary STL: non-finite coordinate in triangle " + std::to_string(i));
            }
            idx[k] = welder.add(p);
        }
        mesh->addFace(idx[0], idx[1], idx[2]);
    }

    metrics::MetricsRegistry::global().increment("io.stl.binary_files");
    return mesh;
}

// ---------------------------------------------------------------------------
//  ASCII
// ---------------------------------------------------------------------------
class AsciiParser {
public:
    AsciiParser(std::istream& in, const ReadOptions& opt) : tokens_(in), opt_(opt) {}

    Result<std::unique_ptr<Mesh>> parse() {
        auto mesh = std::make_unique<Mesh>();
        detail::VertexWelder welder(*mesh, opt_.weldVertices);

        GK_RETURN_IF_ERROR(expect("solid"));
        mesh->setName(tokens_.restOfLine());

        std::size_t facets = 0;
        std::string_view tok;
        for (;;) {
            if (!tokens_.next(tok)) {
                GK_LOG_WARN("ASCII STL: missing 'endsolid' (accepted {} facet(s))", facets);
                break;
            }
            if (detail::iequals(tok, "endsolid")) {
                break;
            }
            if (!detail::iequals(tok, "facet")) {
                return error("expected 'facet' or 'endsolid', got '" + std::string(tok) + "'");
            }
            if (facets >= opt_.maxTriangles) {
                return makeError(ErrorCode::LimitExceeded, "ASCII STL exceeds the triangle limit of " +
                                                               std::to_string(opt_.maxTriangles));
            }

            Vec3 ignoredNormal;
            GK_RETURN_IF_ERROR(expect("normal"));
            GK_RETURN_IF_ERROR(readVec3(ignoredNormal));
            GK_RETURN_IF_ERROR(expect("outer"));
            GK_RETURN_IF_ERROR(expect("loop"));

            std::array<Mesh::Index, 3> idx{};
            for (auto& index : idx) {
                Vec3 p;
                GK_RETURN_IF_ERROR(expect("vertex"));
                GK_RETURN_IF_ERROR(readVec3(p));
                if (!p.isFinite()) {
                    return error("non-finite vertex coordinate");
                }
                index = welder.add(p);
            }
            GK_RETURN_IF_ERROR(expect("endloop"));
            GK_RETURN_IF_ERROR(expect("endfacet"));

            mesh->addFace(idx[0], idx[1], idx[2]);
            ++facets;
        }

        metrics::MetricsRegistry::global().increment("io.stl.ascii_files");
        return mesh;
    }

private:
    core::Error error(const std::string& what) const {
        return makeError(ErrorCode::ParseError,
                         "ASCII STL line " + std::to_string(tokens_.line()) + ": " + what);
    }

    Status expect(std::string_view keyword) {
        std::string_view tok;
        if (!tokens_.next(tok)) {
            return error("unexpected end of file, expected '" + std::string(keyword) + "'");
        }
        if (!detail::iequals(tok, keyword)) {
            return error("expected '" + std::string(keyword) + "', got '" + std::string(tok) + "'");
        }
        return Status::success();
    }

    Status readVec3(Vec3& out) {
        std::array<double, 3> v{};
        for (auto& component : v) {
            std::string_view tok;
            if (!tokens_.next(tok)) {
                return error("unexpected end of file while reading a number");
            }
            if (!detail::parseDouble(tok, component)) {
                return error("invalid number '" + std::string(tok) + "'");
            }
        }
        out = {v[0], v[1], v[2]};
        return Status::success();
    }

    detail::TokenStream tokens_;
    const ReadOptions& opt_;
};

}  // namespace

// ---------------------------------------------------------------------------

core::Result<std::unique_ptr<Mesh>> StlReader::read(std::istream& in, const ReadOptions& options) const {
    if (!in) {
        return makeError(ErrorCode::IoError, "STL: input stream is not readable");
    }
    const auto size = detail::remainingBytes(in);
    if (!size) {
        return makeError(ErrorCode::IoError, "STL: input stream must be seekable");
    }

    Result<std::unique_ptr<Mesh>> result = makeError(ErrorCode::Internal, "unreachable");
    switch (detect(in, *size)) {
        case StlKind::Binary: result = readBinary(in, *size, options); break;
        case StlKind::Ascii:  result = AsciiParser(in, options).parse(); break;
        case StlKind::Invalid:
            return makeError(ErrorCode::ParseError,
                             "STL: stream too small for binary and has no 'solid' keyword");
    }
    if (!result) {
        return result;
    }

    auto& mesh = result.value();
    if (options.validate) {
        GK_RETURN_IF_ERROR(mesh->validate());
    }
    if (const auto degenerate = mesh->countDegenerateFaces(); degenerate > 0) {
        GK_LOG_WARN("STL: {} degenerate face(s) in '{}'", degenerate, mesh->name());
    }
    metrics::MetricsRegistry::global().increment("io.stl.triangles", mesh->faceCount());
    GK_LOG_DEBUG("STL: read {} faces / {} vertices", mesh->faceCount(), mesh->vertexCount());
    return result;
}

}  // namespace gk::io
