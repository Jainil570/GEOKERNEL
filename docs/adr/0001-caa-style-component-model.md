# 0001. CAA-style component model with `query_interface`

* Status: Accepted
* Date: 2026-11-01

## Context

The library must let users swap algorithms (e.g. ray/triangle kernels) and add file formats without modifying or recompiling existing code. Industrial CAD platforms (CATIA's CAA, COM) solve this with interface-based components discovered at runtime. We want the same extensibility with modern C++ safety.

## Decision

* Every capability is an abstract interface deriving **virtually** from `gk::core::IBase`, with a stable string identifier hashed at compile time (`GK_DECLARE_INTERFACE`).
* Components implement one or more interfaces via the CRTP base `Component<Self, I...>`, which generates `queryInterface` with a fold expression.
* Clients navigate between interfaces with `query_interface<I>()`, which returns a `std::shared_ptr<I>` built with the aliasing constructor so it shares ownership of the whole component.

## Consequences

* + New implementations plug in through the registry; client code depends only on interfaces.
* + No manual reference counting; lifetime is correct by construction.
* + Interface identity does not depend on RTTI, so it remains valid across shared-library boundaries.
* − One virtual base per component (an extra pointer and a slightly more expensive upcast). Irrelevant at component granularity; hot-path value types (`Vec3`, `Mesh`) are deliberately *not* components.
* − Interface names are ABI: renaming one is a breaking change.

## Alternatives considered

* **`dynamic_cast` to interfaces** — works in-process but couples identity to RTTI, which is unreliable across DLLs and unavailable with `-fno-rtti`.
* **`std::variant` of known implementations** — closed set; violates the "extend without modifying" goal.
* **Templates/concepts (static polymorphism)** — zero overhead but no runtime selection or discovery.
