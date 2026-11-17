# Tech Stack

Every tool in the project, what it does here, and why it was chosen over the alternatives.

## Language and runtime

| Item | Version | Role | Why |
|---|---|---|---|
| **C++** | 17 (`CMAKE_CXX_EXTENSIONS OFF`) | Implementation language | Fold expressions (interface tables), `if constexpr`, `std::optional` / `std::variant` / `std::string_view` / `std::filesystem` / `std::shared_mutex` — without requiring C++20 toolchains still common in CAD/CAE shops |
| **Standard library only** in public headers | — | API surface | Consumers never inherit a third-party ABI or version constraint |
| Smart pointers | `std::unique_ptr`, `std::shared_ptr` (aliasing ctor) | Ownership & component lifetime | Replaces CAA's manual `AddRef`/`Release`; leaks and double-frees are structurally impossible |

## Build and packaging

| Tool | Version | Role | Notes |
|---|---|---|---|
| **CMake** | ≥ 3.20 (presets need 3.21) | Build system generator | Target-based usage requirements; `GeoKernel::geokernel` exported for `find_package` |
| **CMake Presets** | schema v3 | One-command configs | `dev`, `release`, `asan`, `coverage`, `tidy`, `msvc` — identical locally and in CI |
| **Ninja** | any | Build executor | Fast incremental builds; default generator in presets |
| **FetchContent** | built-in | Dependency acquisition | Pinned release tarballs; `find_package` is tried first so vcpkg/Conan/system packages win |
| **GenerateExportHeader** | built-in | `GK_EXPORT` macros | Same sources build as static or shared (`BUILD_SHARED_LIBS`); hidden visibility by default |
| **CMakePackageConfigHelpers** | built-in | `GeoKernelConfig.cmake` + version file | `SameMajorVersion` compatibility |
| **CPack** | built-in | Release archives | TGZ (Linux/macOS), ZIP (Windows) attached to GitHub Releases |

## Libraries

| Library | Version | Scope | Role | Why |
|---|---|---|---|---|
| **spdlog** | 1.14.1 | `PRIVATE`, header-only, `BUILD_INTERFACE` | Structured, leveled, thread-safe logging | De-facto standard, fmt-style formatting, runtime-addable sinks. Kept out of the public API and the exported link interface so it can never conflict with a host app's spdlog/fmt |
| **GoogleTest / GoogleMock** | 1.15.2 | tests only | Unit, parameterised (`TEST_P`) and integration tests | `gtest_discover_tests` gives one CTest entry per case; value-parameterised suites test the *interface contract* against every implementation |
| **Google Benchmark** | 1.9.0 | benchmarks only (`GK_BUILD_BENCHMARKS`) | Micro-benchmarks of the intersection hot path | Statistically robust timing, custom counters (hit rate, triangle count) |

## Quality and verification

| Tool | Where | What it guarantees |
|---|---|---|
| **Compiler warnings as errors** | all targets we own | `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast -Wdouble-promotion …` (GCC/Clang), `/W4 /permissive-` (MSVC). Third-party headers are marked `SYSTEM` |
| **AddressSanitizer** | `asan` preset, CI job | Heap/stack/global overflows, use-after-free, use-after-return, leaks (`detect_leaks=1`) |
| **UndefinedBehaviorSanitizer** | `asan` preset, CI job | Signed overflow, invalid shifts, misaligned access, bad casts; `-fno-sanitize-recover=all` makes any finding fatal |
| **clang-tidy** | `tidy` preset, CI job | `bugprone-*`, `cert-*`, `cppcoreguidelines-*`, `concurrency-*`, `performance-*`, `modernize-*`, naming conventions — see `.clang-tidy` |
| **clang-format** | CI job, `scripts/format.sh` | One canonical style (`.clang-format`), checked with `--dry-run --Werror` |
| **gcov + gcovr** | `coverage` preset, CI job | Line + branch coverage of `src/` and `include/` only; build fails below **85 %** lines |
| **Codecov** | CI upload | Coverage trend and per-PR diff coverage (`codecov.yml`) |
| **CTest** | everywhere | Unit (`unit` label), CLI/example smoke tests (`smoke` label), sanitizer env via test presets |

## CI/CD — GitHub Actions

| Workflow | Jobs |
|---|---|
| `ci.yml` (push/PR) | clang-format · build+test on Linux GCC 13, Linux Clang 18, Linux Release, macOS AppleClang, Windows MSVC 2022 · ASan+UBSan · coverage gate + Codecov · clang-tidy · install + `find_package` consumer |
| `release.yml` (tag `v*.*.*`) | build + test + CPack on 3 OSes → GitHub Release with generated notes |
| `dependabot.yml` | weekly GitHub Actions version bumps |

## Developer environment

| Item | Purpose |
|---|---|
| `Dockerfile` | Ubuntu 24.04 image with GCC 13, Clang 18 (+ tidy/format), CMake, Ninja, gcovr, Doxygen — reproduces CI exactly |
| `.devcontainer/` | VS Code / Codespaces one-click environment on the same image |
| `.editorconfig`, `.gitattributes` | Consistent whitespace / line endings; byte-exact mesh fixtures |
| **Doxygen** (+ Graphviz) | `-DGK_BUILD_DOCS=ON` → `docs` target renders the API reference from header comments |

## Supported toolchains

| Compiler | Minimum | Tested in CI |
|---|---|---|
| GCC | 9 (8 with `stdc++fs`, handled automatically) | 13 |
| Clang | 10 | 18 |
| AppleClang | 13 | macOS 14 runner |
| MSVC | 19.29 (VS 2019 16.10) | VS 2022 |
