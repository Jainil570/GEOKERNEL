# Security Policy

GeoKernel parses untrusted binary and text files, so memory-safety and resource-exhaustion bugs in the readers are treated as security issues.

## Supported versions

| Version | Supported |
|---|---|
| 1.x | ✅ |
| < 1.0 | ❌ |

## Reporting a vulnerability

Please **do not open a public issue**. Use GitHub's private vulnerability reporting ("Security" tab → "Report a vulnerability") with:

* a description of the issue and its impact,
* a minimal reproducing input file (or a script that generates it),
* the GeoKernel version, platform and compiler.

You will receive an acknowledgement within 5 working days. Fixes are released as patch versions and credited in `CHANGELOG.md` unless you prefer otherwise.

## Hardening already in place

* Declared element counts are validated against the actual stream size **before** allocation.
* `ReadOptions::maxTriangles` bounds memory use for every format.
* Non-finite coordinates and out-of-range indices are rejected with an `Error`.
* All tests run under AddressSanitizer + UndefinedBehaviorSanitizer in CI; clang-tidy `cert-*` and `bugprone-*` checks are enforced.
