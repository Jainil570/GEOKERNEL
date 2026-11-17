#!/usr/bin/env bash
# Produce line/branch coverage for the library and enforce the CI gate.
#   cmake --preset coverage && cmake --build --preset coverage && ctest --preset coverage
#   scripts/coverage.sh [build-dir]          (default: build/coverage)
# Env: GCOV=gcov-13 to pick a specific gcov; COVERAGE_MIN=85 to change the gate.
set -euo pipefail

BUILD_DIR="${1:-build/coverage}"
MIN="${COVERAGE_MIN:-85}"
GCOV_EXE="${GCOV:-gcov}"
cd "$(dirname "$0")/.."

mkdir -p "$BUILD_DIR/coverage-html"

# Only library code counts: tests, examples, apps and third-party are excluded.
gcovr \
  --root . \
  --gcov-executable "$GCOV_EXE" \
  --filter 'src/' \
  --filter 'include/geokernel/' \
  --exclude-throw-branches \
  --exclude-unreachable-branches \
  --print-summary \
  --xml "$BUILD_DIR/coverage.xml" \
  --html-details "$BUILD_DIR/coverage-html/index.html" \
  --fail-under-line "$MIN" \
  "$BUILD_DIR"

echo "HTML report: $BUILD_DIR/coverage-html/index.html"
