#include "geokernel/geometry/Mesh.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace gk::geometry {

Mesh::Mesh(std::vector<Vec3> vertices, std::vector<Face> faces, std::string name)
    : vertices_(std::move(vertices)), faces_(std::move(faces)), name_(std::move(name)) {
    recomputeBounds();
}

Mesh::Index Mesh::addVertex(const Vec3& p) {
    if (vertices_.size() >= std::numeric_limits<Index>::max()) {
        throw std::length_error("Mesh::addVertex: vertex index space exhausted");
    }
    vertices_.push_back(p);
    bounds_.expand(p);
    return static_cast<Index>(vertices_.size() - 1);
}

void Mesh::addFace(Index a, Index b, Index c) {
    const auto n = vertices_.size();
    if (a >= n || b >= n || c >= n) {
        throw std::out_of_range("Mesh::addFace: index out of range (" + std::to_string(a) + ", " +
                                std::to_string(b) + ", " + std::to_string(c) +
                                ") with vertexCount=" + std::to_string(n));
    }
    faces_.push_back({a, b, c});
}

void Mesh::reserve(std::size_t vertexCount, std::size_t faceCount) {
    vertices_.reserve(vertexCount);
    faces_.reserve(faceCount);
}

void Mesh::clear() noexcept {
    vertices_.clear();
    faces_.clear();
    bounds_ = AABB{};
}

Triangle Mesh::triangle(std::size_t faceIndex) const {
    const Face& f = faces_.at(faceIndex);
    return {vertices_.at(f[0]), vertices_.at(f[1]), vertices_.at(f[2])};
}

double Mesh::surfaceArea() const noexcept {
    double total = 0.0;
    for (const auto& f : faces_) {
        total += Triangle{vertices_[f[0]], vertices_[f[1]], vertices_[f[2]]}.area();
    }
    return total;
}

std::size_t Mesh::countDegenerateFaces(double areaTolerance) const noexcept {
    std::size_t count = 0;
    for (const auto& f : faces_) {
        const bool repeated = f[0] == f[1] || f[1] == f[2] || f[0] == f[2];
        if (repeated ||
            Triangle{vertices_[f[0]], vertices_[f[1]], vertices_[f[2]]}.isDegenerate(areaTolerance)) {
            ++count;
        }
    }
    return count;
}

std::vector<Vec3> Mesh::computeFaceNormals() const {
    std::vector<Vec3> normals;
    normals.reserve(faces_.size());
    for (const auto& f : faces_) {
        normals.push_back(Triangle{vertices_[f[0]], vertices_[f[1]], vertices_[f[2]]}.normal());
    }
    return normals;
}

std::vector<Vec3> Mesh::computeVertexNormals() const {
    std::vector<Vec3> normals(vertices_.size(), Vec3::zero());
    for (const auto& f : faces_) {
        // Unnormalised face normal has length 2*area -> implicit area weighting.
        const Vec3 n = Triangle{vertices_[f[0]], vertices_[f[1]], vertices_[f[2]]}.unnormalizedNormal();
        normals[f[0]] += n;
        normals[f[1]] += n;
        normals[f[2]] += n;
    }
    for (auto& n : normals) {
        n = n.normalized();
    }
    return normals;
}

std::size_t Mesh::memoryFootprint() const noexcept {
    return vertices_.capacity() * sizeof(Vec3) + faces_.capacity() * sizeof(Face) + name_.capacity();
}

core::Status Mesh::validate() const {
    using core::ErrorCode;
    for (std::size_t i = 0; i < vertices_.size(); ++i) {
        if (!vertices_[i].isFinite()) {
            return core::makeError(ErrorCode::InvalidArgument,
                                   "vertex " + std::to_string(i) + " has a non-finite coordinate");
        }
    }
    const auto n = vertices_.size();
    for (std::size_t i = 0; i < faces_.size(); ++i) {
        for (const Index idx : faces_[i]) {
            if (idx >= n) {
                return core::makeError(ErrorCode::InvalidArgument,
                                       "face " + std::to_string(i) + " references vertex " +
                                           std::to_string(idx) + " but vertexCount=" + std::to_string(n));
            }
        }
    }
    return core::Status::success();
}

void Mesh::transform(const math::Transform& xf) {
    for (auto& v : vertices_) {
        v = xf.applyToPoint(v);
    }
    if (xf.flipsOrientation()) {
        // A mirror would turn outward normals inward: restore CCW winding.
        for (auto& f : faces_) {
            std::swap(f[1], f[2]);
        }
    }
    recomputeBounds();
}

Mesh Mesh::transformed(const math::Transform& xf) const {
    Mesh copy(*this);
    copy.transform(xf);
    return copy;
}

void Mesh::recomputeBounds() noexcept {
    bounds_ = AABB::fromPoints(vertices_);
}

}  // namespace gk::geometry
