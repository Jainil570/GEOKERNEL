#!/usr/bin/env bash
# Apply (default) or check (--check) clang-format on all C++ sources.
set -euo pipefail
cd "$(dirname "$0")/.."

CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
mapfile -t FILES < <(find include src tests apps examples benchmarks -type f \( -name '*.hpp' -o -name '*.cpp' \))

if [[ "${1:-}" == "--check" ]]; then
  "$CLANG_FORMAT" --dry-run --Werror "${FILES[@]}"
  echo "format OK (${#FILES[@]} files)"
else
  "$CLANG_FORMAT" -i "${FILES[@]}"
  echo "formatted ${#FILES[@]} files"
fi
