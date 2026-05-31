# DSA Mini-Project: Project Restructure

Date: 2026-05-31

## Goal

Restructure the Railway Network Management System project so it's clean, navigable, and easy to present during a live demo + code walkthrough with professors.

## Problem

The project has a split layout — source files at root level, but the build system, README, docs, and viva prep notes inside a directory named `dont_touch/`. The `run.sh` script uses a symlink hack (`ln -sf ../web web`) to bridge the two halves. This is confusing during a walkthrough because:

- CMakeLists.txt is inside `dont_touch/` with `../src/` path prefixes
- README and viva prep notes are not visible at root level
- The name `dont_touch/` is unclear and unprofessional for presentation

## Target Layout

```
DSA-Mini-Project/
├── CMakeLists.txt          # moved from dont_touch/, paths updated (../src/ → src/)
├── README.md               # moved from dont_touch/
├── HOW_TO_RUN.md           # moved from dont_touch/
├── run.sh                  # simplified: no cd/symlink, cmake at root
├── external/               # httplib.h (unchanged)
├── include/                # 11 headers (unchanged)
├── src/                    # 11 source files (unchanged)
├── web/                    # frontend (unchanged)
├── tests/                  # 6 test files (unchanged)
├── notes/                  # moved from dont_touch/notes/, renamed
│   ├── 01-station-records.md
│   ├── 02-waitlist-queue-refund-stack.md
│   ├── 03-dijkstra-prim-pricing.md
│   └── 04-analysis-sort-search-reports.md
└── docs/
    └── superpowers/
        ├── specs/          # design docs (existing + new)
        └── plans/          # implementation plans
```

## Changes

1. **CMakeLists.txt** — move to root `./CMakeLists.txt`, replace all `../` path prefixes with `.`
2. **README.md** — move to root `./README.md`
3. **HOW_TO_RUN.md** — move to root `./HOW_TO_RUN.md`
4. **notes/** — move `dont_touch/notes/*.md` to `./notes/` with numbered prefixes
5. **run.sh** — rewrite: remove `cd dont_touch/`, remove symlink hack, run cmake at root
6. **dont_touch/** — delete entire directory after files are moved
7. **Stale root symlinks** — clean up any orphaned symlink files

## Walkthrough Narrative

With the new layout, the presentation flow becomes:

1. **Root README** — "Here's what the project does and what DSA concepts it covers"
2. **notes/** — "Here are the 4 areas of the project, each with a viva-ready explanation"
3. **include/ → src/** — "Headers here, implementations here, one-to-one mapping"
4. **CMakeLists.txt** — "Standard C++17 project, one command to build"
5. **run.sh → demo** — `./run.sh`, open browser, show the live app

## Files Not Changed

- All source code in `src/` and `include/` — unchanged
- All tests in `tests/` — unchanged
- All web frontend files in `web/` — unchanged
- `external/httplib.h` — unchanged

## Verification

After restructure:
- `./run.sh` starts the server on port 8080
- `ctest --test-dir build -V` runs all tests
- No broken paths or stale symlinks
