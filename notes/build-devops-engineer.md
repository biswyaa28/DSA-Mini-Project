# Viva Preparation Notes — Build & DevOps Engineer

## Your Role
You set up the build system, managed dependencies, and made the project easy to build and run.

## Key Files You Own

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Build configuration |
| `run.sh` | One-command build + run |
| `external/httplib.h` | Bundled third-party dependency |
| `.gitignore` | Ignored files (build/, .worktrees/, data/, .DS_Store) |

---

## Build System: CMake

### Why CMake?
- Industry standard for C++ projects
- Cross-platform (macOS, Linux, Windows)
- Handles compiler detection, linker flags, test registration
- No need to write Makefiles by hand

### CMakeLists.txt Breakdown

```cmake
cmake_minimum_required(VERSION 3.16)
project(railway_app CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

**C++17**: Why this standard?
- `std::optional` available (though we use pointers instead)
- `std::filesystem` available (though not used — we use raw file streams)
- Good compiler support (Clang 5+, GCC 7+, MSVC 2017+)

```cmake
# Header-only libraries — just add include path
include_directories(external)
```

**Header-only libraries**: `cpp-httplib.h` and `nlohmann/json.hpp` are included by just adding `external/` to the include path. No linking needed.

```cmake
# Static library with all engine modules
add_library(railway_core STATIC
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

# Main executable
add_executable(railway_app src/main.cpp src/api_server.cpp)
target_link_libraries(railway_app railway_core)

# Test executables
add_executable(test_models tests/test_models.cpp)
target_link_libraries(test_models railway_core)
# ... (same for all 6 tests)
```

**Why a static library?** Separates engine code from application code. Tests link against the library without needing to compile engine source files again. Faster incremental builds.

---

## The build directory structure

```
After cmake -S . -B build:
  build/
  ├── CMakeCache.txt       ← Cached CMake variables
  ├── railway_app          ← Main executable
  ├── test_models          ← Test executables (6 total)
  ├── librailway_core.a    ← Static library
  └── ... (intermediate files)
```

### Two-Phase Build
1. **Configure**: `cmake -S . -B build` — detects compiler, creates Makefiles
2. **Build**: `cmake --build build` — compiles + links

---

## run.sh Script

```bash
#!/usr/bin/env bash
set -e
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >/dev/null 2>&1
cmake --build build 2>/dev/null
./build/railway_app
```

**What it does:**
1. Configures in Release mode (optimized, no debug symbols)
2. Compiles (suppresses stdout/stderr for cleaner output)
3. Starts the server

**Why `-DCMAKE_BUILD_TYPE=Release`?** Enables compiler optimizations (-O3). Debug mode would be slower and produce more output.

---

## Dependencies

### cpp-httplib (external/httplib.h)
- **Purpose**: HTTP server + client in a single header file
- **Why this library?** Zero dependencies, easy to bundle, well-maintained, supports REST APIs cleanly
- **How it's used**: `httplib::Server` for the API, `httplib::Client` in tests

### nlohmann/json
- **Purpose**: JSON parsing and serialization
- **Why this library?** Most popular C++ JSON library. Expressive syntax (`json j = {{"key", value}}`). Header-only.
- **How it's used**: Parse request bodies, build response objects, serialize bookings to files

### Where are these libraries?
They're in `external/` — but actually, only `httplib.h` is there. The `nlohmann/json.hpp` is included via CMake's `find_package` or bundled similarly. Let's check...

Actually, `nlohmann/json.hpp` is also header-only and included via the `external/` include path. Both are bundled in the repo — no install step needed.

---

## Gitignore

```
build/       # Compiled output (generated)
.worktrees/  # Git worktrees
data/        # Runtime JSON storage
.DS_Store    # macOS metadata
```

---

## Common Viva Questions

**Q: Why CMake instead of Makefiles?** CMake is cross-platform and handles compiler detection automatically. A raw Makefile would need different versions for macOS (Clang), Linux (GCC), and Windows (MSVC).

**Q: Why is the project structured as a static library + executable?** Separation of concerns. The railway_core library contains all business logic. The railway_app executable is just the entry point and API server. Tests link against the library directly — they test the same code that powers the API.

**Q: What happens if I run `cmake --build build` without running `cmake -S . -B build` first?** It fails with "CMakeCache.txt not found." The configure step must run first. The `run.sh` script handles both.

**Q: How do I add a new source file?**
1. Create the .cpp and .hpp files
2. Add the .cpp to `add_library(railway_core ...)` in CMakeLists.txt
3. Rebuild

**Q: Why is nlohmann/json not listed explicitly in CMakeLists.txt?** Because it's header-only and found via the `include_directories(external)` directive. The `#include <nlohmann/json.hpp>` in the source files resolves because `external/` is on the include path.

**Q: How do you run a single test without CTest?** `./build/test_models` — each test is a standalone executable.

**Q: How would you containerize this?**
```dockerfile
FROM gcc:latest
COPY . /app
WORKDIR /app
RUN cmake -S . -B build && cmake --build build
EXPOSE 8080
CMD ["./build/railway_app"]
```

**Q: What if the project needs a database?** Replace `StorageManager` (JSON file) with SQLite. Add `find_package(SQLite3 REQUIRED)` and `target_link_libraries(railway_core SQLite::SQLite3)` to CMakeLists.txt.
