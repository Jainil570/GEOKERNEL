# 0002. Explicit registration through a composition root

* Status: Accepted
* Date: 2026-11-01

## Context

Components must be registered before they can be discovered. A popular pattern is self-registration: each `.cpp` defines a static object whose constructor registers the component (`REGISTER_COMPONENT(X)`).

## Decision

Built-in components are registered explicitly by `gk::registerBuiltinComponents(ComponentRegistry&)` in `src/Kernel.cpp` — the single composition root. `gk::defaultRegistry()` calls it once, lazily and thread-safely (function-local static). Third parties register their components the same way, into the default registry or their own instance.

## Consequences

* + Works identically for static and shared builds: with static libraries, linkers discard object files nobody references, silently dropping self-registering components.
* + No static-initialisation-order fiasco (registry vs. registrants in different TUs).
* + Tests can create isolated registries (dependency injection) and register exactly what they need.
* − Adding a built-in requires one line in `Kernel.cpp` (checked in the PR template).

## Alternatives considered

* **Static self-registration objects** — rejected for the dead-stripping and init-order reasons above; workarounds (`--whole-archive`, `/WHOLEARCHIVE`, `__attribute__((used))`) are toolchain-specific and leak into every consumer's build.
* **Runtime plugin loading (`dlopen`/`LoadLibrary`)** — deferred to the roadmap; it requires an ABI-version handshake and same-toolchain guarantees.
