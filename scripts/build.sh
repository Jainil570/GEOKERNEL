#!/usr/bin/env bash
# Configure, build and test with a CMake preset.
#   scripts/build.sh [preset]        (default: dev)
#   scripts/build.sh asan
set -euo pipefail

PRESET="${1:-dev}"
cd "$(dirname "$0")/.."

cmake --preset "$PRESET"
cmake --build --preset "$PRESET" --parallel
ctest --preset "$PRESET"
