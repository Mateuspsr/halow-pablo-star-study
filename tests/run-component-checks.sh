#!/usr/bin/env bash
set -euo pipefail
study_dir="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$study_dir/validation/raw-membership"
mkdir -p "$build_dir"
g++ -std=c++11 -Wall -Wextra -Werror -pedantic -O2 \
  "$study_dir/tests/raw-membership-check.cc" -o "$build_dir/raw-membership-check"
"$build_dir/raw-membership-check" | tee "$build_dir/run.log"
bash "$study_dir/tests/run-link-quality-check.sh" | tee "$study_dir/validation/link-quality-run.log"
