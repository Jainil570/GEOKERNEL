# Coding Standards

Mechanical rules are enforced by tools (`.clang-format`, `.clang-tidy`, compiler warnings). This page records the *judgement* rules reviewers check.

## Naming

| Entity | Style | Example |
|---|---|---|
| Namespaces | `lower_case` | `gk::geometry` |
| Types | `CamelCase` | `MeshRaycaster` |
| Interfaces | `I` + `CamelCase` | `IMeshReader` |
| Functions / methods / variables | `camelBack` | `closestHit`, `faceCount` |
| Private data members | `camelBack_` | `vertices_` |
| Constants (`constexpr`) | `k` + `CamelCase` | `kLinearTolerance` |
| Macros | `GK_UPPER_CASE` | `GK_DECLARE_INTERFACE` |
| Component names | reverse-DNS string | `"gk.io.StlReader"` |

## Headers

* `#pragma once`; every public header is self-contained (compiles on its own).
* Include order (enforced by clang-format): test framework → standard library → third-party → project.
* Public headers include **only** the standard library and other GeoKernel headers. Third-party includes (spdlog) are allowed only in `src/`.
* Document every public declaration with a `///` Doxygen comment stating contract, units and failure behaviour.

## Interfaces and components

* An interface is a pure abstract class deriving **virtually** from `gk::core::IBase`, declared with `GK_DECLARE_INTERFACE("gk.<module>.<IName>")`. Its name is part of the ABI — never rename it.
* Concrete components derive from `gk::core::Component<Self, Interfaces...>`, are `final`, and declare `static constexpr std::string_view kComponentName`.
* Clients obtain capabilities with `query_interface<I>()` or `registry.createAs<I>()`, never `dynamic_cast` to a concrete type.
* New built-ins are registered **only** in `src/Kernel.cpp`.

## Errors

* External input can fail → return `core::Result<T>` / `core::Status`. Include context (line number, triangle index, path) in the message.
* Broken preconditions → throw a standard exception (`std::invalid_argument`, `std::out_of_range`) and document it with `@throws`.
* Hot-path math → `noexcept`, communicate "no result" with `std::optional`.
* Never let an exception escape a destructor, a `noexcept` function, or a component factory.

## Numerics

* Use the named tolerances in `math/Tolerance.hpp`; no literal epsilons in algorithms.
* Compare geometry with `almostEqual`, never `==` (exact `==` is only for bit-identity, e.g. vertex welding).
* State units and conventions (column vectors, CCW winding, unnormalised ray directions) in the header that owns them.

## Ownership and concurrency

* Return `std::unique_ptr` for newly created heavy objects; accept `std::shared_ptr<const T>` when you need to share immutable data.
* No raw `new`/`delete`. No owning raw pointers.
* `const` methods must be safe to call concurrently. If a type needs a mutable cache, it must be synchronised — prefer maintaining state eagerly instead.
* Instrument per *operation*, never per primitive; cache `Counter&` handles in hot code.

## Tests

* Every bug fix starts with a failing test.
* Test the **interface contract** with value-parameterised tests (`TEST_P`) so every implementation is covered.
* Avoid exact edge cases in geometric tests unless the edge case *is* the test (non-watertight kernels may legitimately differ there).
* Use fixed RNG seeds; tests must be deterministic.
