# GeoKernel

[![CI](https://github.com/Jainil570/GeoKernel/actions/workflows/ci.yml/badge.svg)](https://github.com/Jainil570/GeoKernel/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/Jainil570/GeoKernel/branch/main/graph/badge.svg)](https://codecov.io/gh/Jainil570/GeoKernel)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)

A component-based **C++17** geometry library. Algorithms and file formats live behind abstract
interfaces and are discovered at runtime through a **plugin registry**, in the style of
Dassault Systèmes' CAA (Component Application Architecture).

```cpp
#include <geokernel/geokernel.hpp>

auto mesh = gk::loadMesh("part.stl");                       // STL (binary/ASCII) or OBJ
if (!mesh) { std::cerr << mesh.error().toString(); return 1; }

std::shared_ptr<const gk::geometry::Mesh> part = std::move(mesh).value();
auto kernel = gk::defaultRegistry()                          // pick an algorithm by name
                  .createAs<gk::algorithms::IRayTriangleIntersector>("gk.algorithms.MollerTrumbore");

gk::algorithms::MeshRaycaster raycaster(part, kernel);
if (auto hit = raycaster.closestHit({{0, 0, 10}, {0, 0, -1}}))
    std::cout << "hit at t=" << hit->t << " on face " << hit->faceIndex << '\n';
```

## Features

| Area | What you get |
|---|---|
| **Component model** | `IBase` / `query_interface<I>()`, compile-time interface IDs, CRTP `Component<>` that generates the interface table, shared-ownership interface pointers |
| **Plugin registry** | Thread-safe `ComponentRegistry`: register factories by name, discover by interface, add formats/algorithms without touching existing code |
| **Math** | `Vec3`, `Mat4`, `Quat`, invertible affine `Transform` with cached inverse, correct normal transformation, slerp |
| **Geometry** | `Ray`, `Triangle`, `AABB` (slab test), indexed `Mesh` with eager bounds, validation, vertex normals, mirror-safe transforms |
| **Algorithms** | Möller–Trumbore and edge-function ray/triangle kernels (cross-validated), `MeshRaycaster` with closest/any/all-hit queries and back-face culling |
| **I/O** | STL binary + ASCII (size-based detection, vertex welding, allocation-bomb guards), Wavefront OBJ (n-gons, negative indices, all `f` forms) |
| **Diagnostics** | spdlog logging (private dependency), lock-free runtime counters + scoped timers, `GEOKERNEL_LOG_LEVEL` env var |
| **Quality gates** | 100+ GoogleTest cases, ≥ 85 % line coverage gate, ASan + UBSan, clang-tidy, clang-format, 5-platform CI |

## Quick start

Requirements: CMake ≥ 3.21 (presets), Ninja, a C++17 compiler (GCC ≥ 9, Clang ≥ 10, MSVC 2019+, AppleClang 13+).
Dependencies (spdlog, GoogleTest, Google Benchmark) are fetched automatically at pinned versions,
or taken from your system/vcpkg if already installed.

```bash
cmake --preset dev              # configure (Debug, tests + examples + CLI)
cmake --build --preset dev      # build
ctest --preset dev              # run all tests

./build/dev/apps/geokernel-cli info tests/data/cube.obj
./build/dev/apps/geokernel-cli --metrics raycast tests/data/cube_ascii.stl 0.5 0.7 -5  0 0 1
```

| Preset | Purpose |
|---|---|
| `dev` | Debug build, everything enabled |
| `release` | Optimised build + benchmarks |
| `asan` | Clang + AddressSanitizer + UndefinedBehaviorSanitizer |
| `coverage` | GCC + gcov instrumentation → `scripts/coverage.sh` |
| `tidy` | clang-tidy runs on every translation unit during the build |
| `msvc` | Visual Studio 2022 solution (Windows) |

## Using GeoKernel from another project

```cmake
find_package(GeoKernel 1.0 REQUIRED)            # after `cmake --install`
target_link_libraries(my_app PRIVATE GeoKernel::geokernel)
```

or, without installing:

```cmake
include(FetchContent)
FetchContent_Declare(geokernel GIT_REPOSITORY https://github.com/Jainil570/GeoKernel.git GIT_TAG v1.0.0)
FetchContent_MakeAvailable(geokernel)
target_link_libraries(my_app PRIVATE GeoKernel::geokernel)
```

## Extending: add a file format in ~30 lines

```cpp
class PlyReader final : public gk::core::Component<PlyReader, gk::io::IMeshReader> {
public:
    static constexpr std::string_view kComponentName = "acme.PlyReader";
    std::string_view formatName() const noexcept override { return "PLY"; }
    std::vector<std::string> fileExtensions() const override { return {".ply"}; }
    gk::core::Result<std::unique_ptr<gk::geometry::Mesh>> read(std::istream&, const gk::io::ReadOptions&) const override;
};

gk::defaultRegistry().registerComponent<PlyReader>("PLY reader", "1.0.0");
auto mesh = gk::loadMesh("scan.ply");   // MeshLoader now finds it by extension
```

See [examples/custom_reader_plugin.cpp](examples/custom_reader_plugin.cpp) for a complete, runnable version.

## Repository layout

```
include/geokernel/   public headers (the API) — core/ math/ geometry/ algorithms/ io/
src/                 implementation + private headers (spdlog lives only here)
tests/               unit/ (per module) + integration/ + data/ fixtures
apps/                geokernel-cli
examples/            runnable usage examples (also run as smoke tests)
benchmarks/          Google Benchmark micro-benchmarks
cmake/               warnings, sanitizers, coverage, tidy, deps, install/export, packaging
docs/                ARCHITECTURE.md, TECH_STACK.md, CODING_STANDARDS.md, adr/
scripts/             build, coverage, format, tidy helpers
.github/             CI (5 platforms + sanitizers + coverage + tidy), release packaging
```

## Documentation

- [Architecture](docs/ARCHITECTURE.md) — layers, component model, data flow, threading, error model
- [Tech stack](docs/TECH_STACK.md) — every tool and library, and why it was chosen
- [Coding standards](docs/CODING_STANDARDS.md)
- [Architecture Decision Records](docs/adr/)
- [Contributing](CONTRIBUTING.md) · [Security](SECURITY.md) · [Changelog](CHANGELOG.md)

## License

MIT — see [LICENSE](LICENSE).
