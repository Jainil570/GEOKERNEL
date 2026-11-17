# Architecture Decision Records

Short, immutable records of significant decisions. To change a decision, add a new ADR that supersedes the old one.

| # | Title | Status |
|---|---|---|
| [0001](0001-caa-style-component-model.md) | CAA-style component model with `query_interface` | Accepted |
| [0002](0002-explicit-component-registration.md) | Explicit registration through a composition root | Accepted |
| [0003](0003-error-handling.md) | `Result<T>` for expected failures, exceptions for contract violations | Accepted |
| [0004](0004-private-logging-dependency.md) | spdlog as a private, header-only dependency | Accepted |
| [0005](0005-dependency-management.md) | `find_package` first, pinned FetchContent fallback | Accepted |

Template: [0000-template.md](0000-template.md)
