## What & why

<!-- One paragraph: the problem and the approach. Link the issue: Fixes #123 -->

## Checklist

- [ ] Unit tests added/updated (coverage gate is 85 % lines)
- [ ] `ctest --preset dev` passes locally
- [ ] `cmake --preset asan && cmake --build --preset asan && ctest --preset asan` is clean
- [ ] `clang-format` applied (`scripts/format.sh`)
- [ ] Public API changes documented (Doxygen comments + `CHANGELOG.md`)
- [ ] New component? Registered in `src/Kernel.cpp` and listed in `docs/ARCHITECTURE.md`
- [ ] Architectural decision? Added an ADR under `docs/adr/`
