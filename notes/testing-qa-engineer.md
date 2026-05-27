# Viva Preparation Notes — Testing / QA Engineer

## Your Role
You wrote and ran the unit tests that verify every module works correctly. You ensure the project is reliable before demo.

## Key Files You Own

| File | Tests | Lines |
|------|-------|-------|
| `tests/test_models.cpp` | Station, Route, Booking data structures | ~50 |
| `tests/test_graph_basics.cpp` | StationDirectory, RailGraph | ~70 |
| `tests/test_booking_pricing.cpp` | BookingEngine, PricingEngine | ~80 |
| `tests/test_routing_mst.cpp` | RoutingEngine, MSTEngine | ~80 |
| `tests/test_sort_search_reports.cpp` | SortingAndSearch, ReportEngine | ~70 |
| `tests/test_api_smoke.cpp` | ApiServer endpoint smoke test | ~100 |

## How Tests Are Built

- CMake scans `tests/` directory
- Each test is a separate executable linked against `railway_core` (static lib)
- Tests are registered with CTest via `enable_testing()` and `add_test()`

## Test Framework

- **Catch2? Google Test?** — Neither. These are plain C++ executables with `assert()` and manual PASS/FAIL output.
- Each test file has its own `main()` that runs test cases and prints results
- CTest captures stdout and reports pass/fail

---

## Test Breakdown

### 1. `test_models.cpp` — Data Structure Tests

```cpp
void testStation() {
  Station s{"S1", "Central"};
  assert(s.id == "S1");
  assert(s.name == "Central");
}

void testRoute() {
  Route r{"R1", "S1", "S2", 10, 100, 50, 25};
  assert(r.availableSeats() == 25);   // 50 - 25 = 25
  assert(r.isSurgePricing() == false); // 50% occupancy, not surge
}
```

**Tests:**
- Station creation and field access
- Route fields + `availableSeats()` math
- Route `isSurgePricing()` at various occupancy levels
- Booking creation with different statuses

### 2. `test_graph_basics.cpp` — Station & Graph Tests

**Tests:**
- Add stations, check duplicates rejected
- Add routes, check duplicates rejected
- `findRoute()` returns correct route
- `outgoingRoutes()` returns routes from a station
- `updateOccupiedSeats()` modifies occupancy
- `allRoutes()`, `allStations()` enumeration

### 3. `test_booking_pricing.cpp` — Booking & Pricing Tests

**Tests:**
- Book a seat → status is Confirmed (if capacity available)
- Book beyond capacity → status is Waitlisted
- VIP bookings prioritized on waitlist promotion
- Cancel a booking → status Cancelled + refund stack updated
- Cancel a confirmed booking → auto-promotes waitlisted
- `PricingEngine::currentFare()` — normal fare
- `PricingEngine::currentFare()` — surge fare (>80% occupancy)

### 4. `test_routing_mst.cpp` — Routing & MST Tests

**Tests:**
- `findPath` with "shortest" mode returns correct path
- `findPath` with "least_crowded" mode prefers less crowded routes
- No path returns empty result
- Same source and destination returns empty path
- `buildMST` returns correct route set
- `buildMST` total distance is minimal

### 5. `test_sort_search_reports.cpp` — Sorting & Reports Tests

**Tests:**
- `sortByRouteDateFare()` sorts correctly (route → date → fare priority)
- `binarySearchFareRange()` finds correct range
- Empty range returns {-1, -1}
- `profitabilityByRoute()` aggregates revenue correctly

### 6. `test_api_smoke.cpp` — API Integration Test

This is the most important test — it spins up the server, sends HTTP requests, and verifies responses.

```cpp
// Server starts on a thread
ApiServer server;
server.configure(8081); // different port for testing
std::thread t([&] { server.run(); });

// HTTP client tests
httplib::Client client("http://localhost:8081");

// Test: GET /api/stations returns seed data
auto res = client.Get("/api/stations");
assert(res->status == 200);
// Parse JSON, verify seed stations exist

// Test: POST /api/stations adds new station
auto res = client.Post("/api/stations", /* body */);
assert(res->status == 200);

// Test: POST /api/bookings
auto res = client.Post("/api/bookings", /* body */);
assert(res->status == 200);

// Shutdown
server.stop();
t.join();
```

**Tests:**
- GET /api/stations returns seeded stations
- POST /api/stations adds new station
- POST /api/bookings creates booking
- GET /api/routes/search with different modes
- GET /api/mst returns valid MST
- GET /api/fare-trend returns projection

---

## Running Tests

```bash
# Run all tests
ctest --test-dir build -V

# Run a single test
./build/test_models
```

All 6 tests pass in under 3 seconds.

---

## Common Viva Questions

**Q: What testing framework did you use?** Plain C++ with `assert()` macros. No external test framework — kept dependencies minimal. CTest handles test discovery and reporting.

**Q: Why is API smoke test important?** It validates the entire stack: HTTP server → routing → JSON parsing → engine logic → JSON response. If this passes, the whole system works end-to-end.

**Q: What edge cases did you test?**
- Duplicate station/route IDs → rejected
- Booking beyond capacity → waitlisted
- Cancelling already-cancelled → rejected
- Empty waitlist promotion → no-op
- No path between stations → empty result
- Start = destination → empty path (0 distance)
- Binary search with no matching fares → {-1, -1}
- Invalid JSON body → 400 response
- Missing parameters → 400 response

**Q: How do you test the fare trend predictor?** The API smoke test calls `GET /api/fare-trend?routeId=R1&days=7` and verifies the response has 7 entries with valid fare values.

**Q: What's not tested?** 
- Concurrent requests (no thread safety tests)
- StorageManager persistence (tests don't write to disk)
- Very large graphs (only 5 stations)
- Network errors/timeouts

**Q: How would you improve test coverage?**
- Add property-based tests (random station/route generation)
- Add performance benchmarks
- Test StorageManager with temp files
- Add stress tests (1000+ bookings)

---

## Test Results (from actual run)

```
100% tests passed, 0 tests failed out of 6

Total Test time (real) = 2.83 sec
```

| Test | Time | Status |
|------|------|--------|
| test_models | 0.46s | PASSED |
| test_graph_basics | 0.50s | PASSED |
| test_booking_pricing | 0.44s | PASSED |
| test_routing_mst | 0.48s | PASSED |
| test_sort_search_reports | 0.43s | PASSED |
| test_api_smoke | 0.51s | PASSED |
