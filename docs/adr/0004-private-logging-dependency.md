# 0004. spdlog as a private, header-only dependency

* Status: Accepted
* Date: 2026-11-03

## Context

The library needs fast, leveled, thread-safe logging. Host applications frequently already use spdlog/fmt — often a different version. Exposing spdlog types in our headers would force our version on them (ODR/ABI conflicts).

## Decision

* Public API: a tiny facade `gk::log` (`setLevel`, `addFileSink`, `write`, `flush`, `parseLevel`) using only standard types.
* Implementation: `src/core/LogInternal.hpp` (private) wraps spdlog in `GK_LOG_*` macros.
* Linking: `$<BUILD_INTERFACE:spdlog::spdlog_header_only>` with `PRIVATE` visibility — spdlog is compiled into our objects and absent from the exported link interface and installed headers.
* The library's logger is *not* registered in spdlog's global registry, so a host logger named "geokernel" cannot collide.
* Default level `warn` (a library must be quiet), overridable via `GEOKERNEL_LOG_LEVEL`.

## Consequences

* + Consumers need nothing but `find_package(GeoKernel)`; verified by the CI install-consumer job.
* + spdlog can be upgraded or replaced without an API break.
* − Slightly longer compile times for the few TUs that log (header-only formatting).
* − Host applications cannot attach their own spdlog sink object directly; they use `addFileSink` or read our output.

## Alternatives considered

* **Public spdlog dependency** — simplest, but leaks versions and types into every consumer.
* **Custom logger** — reinventing thread-safe sinks, rotation and formatting is not a good use of effort.
