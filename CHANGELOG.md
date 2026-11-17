# Changelog

All notable changes are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses [Semantic Versioning](https://semver.org/).

## [Unreleased]

## [1.0.0] - 2026-11-17

### Added
- **Component model**: `IBase`, compile-time `InterfaceId`, CRTP `Component<>`, `query_interface<I>()` with shared ownership.
- **Plugin registry**: thread-safe `ComponentRegistry` with descriptors, interface discovery and contained factory failures; `gk::defaultRegistry()`.
- **Math**: `Vec3`, `Mat4` (cofactor inverse), `Quat` (slerp), invertible affine `Transform` with cached inverse and inverse-transpose normals.
- **Geometry**: `Ray`, `Triangle`, `AABB` (slab test), indexed `Mesh` with validation, vertex normals and mirror-safe transforms.
- **Algorithms**: Möller–Trumbore and edge-function ray/triangle intersectors; `MeshRaycaster` (closest / any / all hits, back-face culling, AABB early-out).
- **I/O**: STL (binary + ASCII, size-based detection, vertex welding), Wavefront OBJ (n-gons, negative indices); `MeshLoader` facade; `gk::loadMesh()`.
- **Diagnostics**: `gk::log` facade over private spdlog; lock-free `MetricsRegistry` and `ScopedTimer`.
- `geokernel-cli` (`info`, `raycast`, `components`, `version`), three examples, Google Benchmark suite.
- Build: CMake presets, install/export package, CPack; CI on 5 configurations with ASan/UBSan, coverage gate (85 %), clang-tidy and clang-format.
