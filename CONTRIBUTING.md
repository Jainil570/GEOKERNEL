# Contributing to GeoKernel

## Setup

```bash
# Option A: local toolchain (CMake >= 3.21, Ninja, GCC/Clang/MSVC)
cmake --preset dev && cmake --build --preset dev && ctest --preset dev

# Option B: identical-to-CI container
docker build -t geokernel-dev .
docker run --rm -it -v "$PWD":/src geokernel-dev scripts/build.sh dev
```

VS Code users can open the folder in the dev container (`.devcontainer/`).

## Workflow

1. Branch from `main`: `feat/<topic>`, `fix/<topic>`, `docs/<topic>`.
2. Write a failing test first for bug fixes.
3. Before pushing, run the same gates CI runs:
   ```bash
   scripts/format.sh                      # clang-format
   scripts/build.sh dev                   # build + all tests
   scripts/build.sh asan                  # AddressSanitizer + UBSan
   cmake --preset tidy && cmake --build --preset tidy   # clang-tidy
   cmake --preset coverage && cmake --build --preset coverage \
     && ctest --preset coverage && scripts/coverage.sh  # >= 85 % lines
   ```
4. Commit messages follow [Conventional Commits](https://www.conventionalcommits.org): `feat(io): add PLY reader`.
5. Open a PR and complete the checklist in the template. CI must be green.

## Adding a component

1. Define (or reuse) an interface in `include/geokernel/<module>/I<Name>.hpp` with `GK_DECLARE_INTERFACE("gk.<module>.I<Name>")`.
2. Implement it as `final : public core::Component<Impl, I<Name>>` with a `kComponentName`.
3. Register it in `src/Kernel.cpp` (`registerBuiltinComponents`) and add its name to `gk::components`.
4. If it implements an existing interface, add it to that interface's parameterised contract test (e.g. `RayTriangleContract`).
5. Document it in `docs/ARCHITECTURE.md` and `CHANGELOG.md`.

## Coding standards

See [docs/CODING_STANDARDS.md](docs/CODING_STANDARDS.md). Significant design changes need an ADR in [docs/adr/](docs/adr/).
