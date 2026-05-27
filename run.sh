#!/usr/bin/env bash
set -e
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >/dev/null 2>&1
cmake --build build 2>/dev/null
./build/railway_app
