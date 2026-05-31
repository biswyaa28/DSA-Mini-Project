# Project Restructure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restructure the DSA Mini-Project from a split layout (root src/ + nested dont_touch/) into a clean flat monorepo.

**Architecture:** Move CMakeLists.txt, README, HOW_TO_RUN, and viva notes from `dont_touch/` to root. Update relative paths. Delete `dont_touch/`. Rewrite `run.sh` to build at root.

**Tech Stack:** C++17, CMake 3.16+, bash

---

### Task 1: Move and update CMakeLists.txt

**Files:**
- Move: `dont_touch/CMakeLists.txt` → `CMakeLists.txt`
- Modify: `CMakeLists.txt` (all path references)

- [ ] **Step 1: Copy CMakeLists.txt to root with updated paths**

Read `dont_touch/CMakeLists.txt` and write a new `CMakeLists.txt` at root with all `../` path prefixes removed:

```cmake
cmake_minimum_required(VERSION 3.16)
project(railway_webui_task1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
  HINTS /opt/homebrew/include /usr/local/include /usr/include
)

find_package(Threads REQUIRED)

add_executable(railway_app src/main.cpp src/api_server.cpp)
target_include_directories(railway_app PRIVATE include external ${NLOHMANN_INCLUDE_DIR})
target_link_libraries(railway_app PRIVATE railway_core Threads::Threads)

add_library(railway_core
  src/station_directory.cpp
  src/rail_graph.cpp
  src/pricing_engine.cpp
  src/booking_engine.cpp
  src/routing_engine.cpp
  src/mst_engine.cpp
  src/sort_search.cpp
  src/report_engine.cpp
  src/storage_manager.cpp
)
target_include_directories(railway_core PUBLIC include ${NLOHMANN_INCLUDE_DIR})

enable_testing()
add_executable(test_models tests/test_models.cpp)
target_include_directories(test_models PRIVATE include)
add_test(NAME test_models COMMAND test_models)

add_executable(test_graph_basics tests/test_graph_basics.cpp)
target_include_directories(test_graph_basics PRIVATE include)
target_link_libraries(test_graph_basics PRIVATE railway_core)
add_test(NAME test_graph_basics COMMAND test_graph_basics)

add_executable(test_booking_pricing tests/test_booking_pricing.cpp)
target_include_directories(test_booking_pricing PRIVATE include)
target_link_libraries(test_booking_pricing PRIVATE railway_core)
add_test(NAME test_booking_pricing COMMAND test_booking_pricing)

add_executable(test_routing_mst tests/test_routing_mst.cpp)
target_include_directories(test_routing_mst PRIVATE include)
target_link_libraries(test_routing_mst PRIVATE railway_core)
add_test(NAME test_routing_mst COMMAND test_routing_mst)

add_executable(test_sort_search_reports tests/test_sort_search_reports.cpp)
target_include_directories(test_sort_search_reports PRIVATE include ${NLOHMANN_INCLUDE_DIR})
target_link_libraries(test_sort_search_reports PRIVATE railway_core)
add_test(NAME test_sort_search_reports COMMAND test_sort_search_reports)

add_executable(test_api_smoke tests/test_api_smoke.cpp src/api_server.cpp)
target_include_directories(test_api_smoke PRIVATE include external ${NLOHMANN_INCLUDE_DIR})
target_link_libraries(test_api_smoke PRIVATE railway_core Threads::Threads)
add_test(NAME test_api_smoke COMMAND test_api_smoke)
```

Changes: `../src/` → `src/`, `../include` → `include`, `../tests/` → `tests/`. The `external` reference had no prefix and stays the same.

- [ ] **Step 2: Verify the file was written correctly**

Run: `cat CMakeLists.txt | head -5`
Expected: shows `cmake_minimum_required(VERSION 3.16)` at line 1, no `../` paths

---

### Task 2: Move README and HOW_TO_RUN to root

**Files:**
- Move: `dont_touch/README.md` → `README.md`
- Move: `dont_touch/HOW_TO_RUN.md` → `HOW_TO_RUN.md`

- [ ] **Step 1: Copy README.md to root**

```bash
cp dont_touch/README.md README.md
```

- [ ] **Step 2: Copy HOW_TO_RUN.md to root**

```bash
cp dont_touch/HOW_TO_RUN.md HOW_TO_RUN.md
```

- [ ] **Step 3: Verify both files at root**

Run: `ls -la README.md HOW_TO_RUN.md`
Expected: both files exist at root level

---

### Task 3: Move and rename viva prep notes

**Files:**
- Move: `dont_touch/notes/p1.md` → `notes/01-station-records.md`
- Move: `dont_touch/notes/p2.md` → `notes/02-waitlist-queue-refund-stack.md`
- Move: `dont_touch/notes/p3.md` → `notes/03-dijkstra-prim-pricing.md`
- Move: `dont_touch/notes/p4.md` → `notes/04-analysis-sort-search-reports.md`

- [ ] **Step 1: Create notes directory at root**

```bash
mkdir -p notes
```

- [ ] **Step 2: Copy and rename all four files**

```bash
cp dont_touch/notes/p1.md notes/01-station-records.md
cp dont_touch/notes/p2.md notes/02-waitlist-queue-refund-stack.md
cp dont_touch/notes/p3.md notes/03-dijkstra-prim-pricing.md
cp dont_touch/notes/p4.md notes/04-analysis-sort-search-reports.md
```

- [ ] **Step 3: Verify notes exist**

Run: `ls -la notes/`
Expected: 4 numbered `.md` files

---

### Task 4: Rewrite run.sh

**Files:**
- Modify: `run.sh` (simplify to build at root, remove symlink hack)

- [ ] **Step 1: Write new run.sh**

```bash
#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >/dev/null 2>&1
cmake --build build 2>/dev/null
exec ./build/railway_app
```

- [ ] **Step 2: Make sure it's executable**

```bash
chmod +x run.sh
```

---

### Task 5: Delete dont_touch/ directory

**Files:**
- Delete: `dont_touch/` (entire directory)

- [ ] **Step 1: Remove dont_touch/ directory**

```bash
rm -rf dont_touch
```

- [ ] **Step 2: Verify it's gone**

Run: `ls dont_touch 2>&1 || echo "GONE"`
Expected: `ls: dont_touch: No such file or directory` followed by `GONE`

---

### Task 6: Clean up stale root-level symlinks

**Files:**
- Check: any stale `web` symlink at root level (not the `web/` directory)

- [ ] **Step 1: Check for stale symlink**

```bash
ls -la web 2>&1
```

If `web` shows as a symlink (e.g., `web -> ...`) rather than a directory, remove it.

- [ ] **Step 2: Verify web/ directory still exists**

Run: `ls web/index.html`
Expected: `web/index.html` (the real directory is intact)

---

### Task 7: Build and test

- [ ] **Step 1: Clean build from root**

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Expected: Configuration done, no errors

- [ ] **Step 2: Build the project**

```bash
cmake --build build
```

Expected: Build succeeds, `build/railway_app` binary exists

- [ ] **Step 3: Run all tests**

```bash
ctest --test-dir build -V
```

Expected: All 6 tests pass

- [ ] **Step 4: Start server and smoke-test**

```bash
./build/railway_app &
sleep 1
curl -s http://localhost:8080/api/stations | head -c 100
kill %1
```

Expected: JSON response with 5 stations (seed data)
