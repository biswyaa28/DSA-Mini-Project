#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >/dev/null 2>&1
cmake --build build 2>/dev/null
exec ./build/railway_app
