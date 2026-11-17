#!/usr/bin/env bash
# Standalone clang-tidy over library sources using an existing compile_commands.json.
#   cmake --preset dev && scripts/run-clang-tidy.sh build/dev
# (Alternative: `cmake --preset tidy` runs clang-tidy as part of compilation.)
set -euo pipefail

BUILD_DIR="${1:-build/dev}"
cd "$(dirname "$0")/.."

if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
  echo "error: $BUILD_DIR/compile_commands.json not found — configure first" >&2
  exit 2
fi

RUNNER="$(command -v run-clang-tidy || command -v run-clang-tidy-18 || true)"
if [[ -n "$RUNNER" ]]; then
  "$RUNNER" -p "$BUILD_DIR" -quiet '^.*/src/.*\.cpp$'
else
  find src -name '*.cpp' -print0 | xargs -0 -n1 -P"$(nproc)" clang-tidy -p "$BUILD_DIR" --quiet
fi
