#!/usr/bin/env bash
set -euo pipefail
study_dir="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$study_dir/validation/link-quality"
mkdir -p "$build_dir"
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -O2 \
  "$study_dir/implementation/link-quality.cc" \
  "$study_dir/tests/link-quality-check.cc" \
  -o "$build_dir/link-quality-check"
"$build_dir/link-quality-check"
