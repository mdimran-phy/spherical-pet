#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ ! -x "$project_dir/build/spherical-pet" ]]; then
    echo 'Build first: cmake -S . -B build && cmake --build build -j4' >&2
    exit 1
fi
exec "$project_dir/build/spherical-pet" "$@"
