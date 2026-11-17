# 0003. `Result<T>` for expected failures, exceptions for contract violations

* Status: Accepted
* Date: 2026-11-02

## Context

Mesh files come from users and other tools; malformed input is *expected*, not exceptional. At the same time, misuse of the API (bad indices, zero scale) is a bug in the caller. C++17 has no `std::expected`.

## Decision

* Expected failures return `gk::core::Result<T>` (value or `Error{ErrorCode, message}`) or `gk::core::Status`. Messages carry location context (line number, triangle index, path).
* Precondition violations throw standard exceptions (`std::invalid_argument`, `std::out_of_range`) documented with `@throws`.
* Hot-path math is `noexcept` and signals "no result" with `std::optional` or neutral values (zero vector, identity).
* Accessing the wrong alternative of a `Result` throws `BadResultAccess` — itself a programming error.

## Consequences

* + Callers cannot accidentally ignore load failures (`[[nodiscard]]` on `Result`/`Status`).
* + No exception overhead or `try` blocks on the normal error path for I/O.
* + `Result` mirrors `std::expected`, so migrating to C++23 is mechanical.
* − Two error channels to learn; the rule above is documented in `CODING_STANDARDS.md`.

## Alternatives considered

* **Exceptions everywhere** — makes "file is malformed" expensive and easy to forget to catch across component boundaries.
* **Error codes + out-parameters** — loses the value/error pairing and is easy to misuse.
* **Third-party `expected` (tl::expected)** — adds a public dependency for ~150 lines of code.
