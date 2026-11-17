# 0005. `find_package` first, pinned FetchContent fallback

* Status: Accepted
* Date: 2026-11-03

## Context

Contributors and CI should build with one command and no pre-installed packages. Packagers (vcpkg, Conan, distros) need to inject their own builds of the same libraries.

## Decision

`cmake/Dependencies.cmake` tries `find_package(<dep> <version> CONFIG)` and, only if that fails, downloads a **pinned release tarball** with `FetchContent`. Versions are variables at the top of the file; bumping one is a reviewed change. Fetched targets have their include directories marked `SYSTEM` so our `-Werror` warning set never fires in third-party headers, and unused targets are excluded from `ALL`.

## Consequences

* + `cmake --preset dev` works on a clean machine; CI caches `_deps/` keyed on the dependency file hash.
* + Package managers override transparently.
* − First configure needs network access (or a pre-populated `FETCHCONTENT_BASE_DIR`).

## Alternatives considered

* **Git submodules** — easy to forget to update/init, bloat clones.
* **Mandatory vcpkg manifest** — excellent, but forces a tool on every contributor; can be added later as an *option* (`vcpkg.json`) without changing this policy.
