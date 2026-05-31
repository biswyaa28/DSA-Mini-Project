# P4: Graph Analysis, Route Analysis & Reports

## Your Role
You are responsible for **graph implementation, route analysis, and generating reports**. You own the analytical layer — sorting, searching, profitability reports, seat occupancy analysis, fare trend prediction, and the API endpoints that serve this data. While P1 owns the raw graph storage, you own the **graph analysis** — extracting insights from the graph data.

## Files You Own

| File | Lines | What it does |
|------|-------|--------------|
| `src/sort_search.cpp` | 59 | Multi-level sorting + binary search on fare |
| `src/report_engine.cpp` | 10 | Route profitability report |
| `src/api_server.cpp` (analysis endpoints) | ~100 | 5 REST endpoints for graph analysis + reports |

You also depend on:
- `include/models.hpp` — `Route`, `Booking` structs (P1)
- `include/rail_graph.hpp` — to access route data for graph-level analysis (P1)
- `include/routing_engine.hpp` — Dijkstra results for route search API (P3)
- `include/mst_engine.hpp` — MST results for reports (P3)
- `include/pricing_engine.hpp` — for fare trend prediction (P3)
- `web/app.js` — frontend rendering of your analysis data

---

## 1. Multi-Level Sorting

### What it does
Sorts bookings by three keys in priority order:
1. **Route ID** (primary) — groups all bookings on the same route
2. **Travel Date** (secondary) — chronological within each route
3. **Charged Fare** (tertiary) — cheapest first within each date

```cpp
void SortingAndSearch::sortByRouteDateFare(std::vector<Booking>& bookings) {
  std::sort(bookings.begin(), bookings.end(),
      [](const Booking& a, const Booking& b) {
        if (a.routeId != b.routeId)
          return a.routeId < b.routeId;          // 1st key: route
        if (a.travelDate != b.travelDate)
          return a.travelDate < b.travelDate;    // 2nd key: date
        return a.chargedFare < b.chargedFare;    // 3rd key: fare
      });
}
```

### How `std::sort` Works
- **Introsort** — hybrid algorithm: starts with Quicksort, switches to Heapsort if recursion depth exceeds O(log N), uses Insertion Sort for small partitions
- Time complexity: O(N log N) worst case (unlike raw Quicksort which can degrade to O(N²))
- In-place sort (modifies original vector)

### Why This Sorting Order?
For a railway report: all R1 bookings together (sorted by ID), then within R1 by date (chronological), then by fare (cheapest first). Makes the data readable and useful for analysis.

---

## 2. Binary Search on Fare Range

### What it does
Given a vector of bookings sorted by fare, find all bookings whose fare falls within [minFare, maxFare].

### Implementation — Two Binary Searches
```cpp
std::pair<int, int> SortingAndSearch::binarySearchFareRange(
    const std::vector<Booking>& sortedFares,
    double minFare, double maxFare) {

  // Binary Search 1: LOWER BOUND — first index with fare >= minFare
  int lo = 0, hi = sortedFares.size() - 1, start = -1;
  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    if (sortedFares[mid].chargedFare >= minFare) {
      start = mid;      // candidate found
      hi = mid - 1;     // search left half for earlier match
    } else {
      lo = mid + 1;     // fare too low, search right
    }
  }

  if (start == -1) return {-1, -1};  // nothing found

  // Binary Search 2: UPPER BOUND — last index with fare <= maxFare
  lo = start;
  hi = sortedFares.size() - 1;
  int end = -1;
  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    if (sortedFares[mid].chargedFare <= maxFare) {
      end = mid;         // candidate found
      lo = mid + 1;      // search right half for later match
    } else {
      hi = mid - 1;      // fare too high, search left
    }
  }

  return {start, end};  // inclusive range [start, end]
}
```

### Why Two Binary Searches?
- First search finds the **start** of the range (lower bound for minFare)
- Second search finds the **end** of the range (upper bound for maxFare)
- Two O(log N) searches = O(log N) total
- A linear scan after finding start would be O(N) worst case

### Why Overflow-Safe Mid Calculation?
```cpp
int mid = lo + (hi - lo) / 2;  // safe
// NOT: (lo + hi) / 2  — can overflow 32-bit int
```
If `lo = 2,000,000,000` and `hi = 2,000,000,000`, then `lo + hi = 4,000,000,000` which overflows a 32-bit signed int. `lo + (hi - lo)/2` is mathematically equivalent but never exceeds `hi`.

### Return Value Convention
`{-1, -1}` means "no bookings found in this fare range." The API checks for this and returns an empty results array to the frontend.

### Why Binary Search Requires Sorted Data
Binary search works by comparing the middle element to the target and discarding half the array each iteration. This ONLY works if the array is sorted — otherwise `fare[mid] < target` gives no information about which half to search next.

---

## 3. Route Profitability Report

### Implementation — Only 10 Lines!
```cpp
std::map<std::string, double> ReportEngine::profitabilityByRoute(
    const std::vector<Booking>& bookings) {
  std::map<std::string, double> report;
  for (const auto& b : bookings) {
    report[b.routeId] += b.chargedFare;  // sum fares per route
  }
  return report;
}
```

### Why `std::map` Here
`std::map` is a **balanced binary search tree** (Red-Black Tree). Keys are stored in sorted order automatically. This means:
- Route IDs appear alphabetically in the report (R1, R2, R3...) — no separate sort step
- Each `report[b.routeId] += fare` is O(log N) — insert if new, update if existing
- `operator[]` creates a 0.0 entry if key doesn't exist, then adds fare
- Total: O(N log N) for N bookings

### `std::map` vs `std::unordered_map`

| Property | `std::map` (ours) | `std::unordered_map` |
|----------|-------------------|---------------------|
| Internal | Red-Black Tree | Hash table |
| Key order | Sorted ascending | Arbitrary |
| Lookup | O(log N) | O(1) average |
| Insert | O(log N) | O(1) average |
| Memory | More (tree pointers) | More (hash table) |

**Why map here?** The report output needs sorted keys. Map gives us sorted order for free. `unordered_map` would require an extra sort step.

### Example Output
```json
{"R1": 450.50, "R2": 300.00, "R3": 650.00}
```

---

## 4. Seat Occupancy Map (Graph Analysis)

### What it is
A route-by-route breakdown of seat occupancy — which routes are busy, which have spare capacity. This is **graph analysis**: overlaying state (occupancy) onto graph edges (routes) to understand usage patterns.

### How It Works (in api_server.cpp)
```cpp
for (const auto& r : graph_.allRoutes()) {
  double pct = r.capacity > 0 ? (100.0 * r.occupiedSeats / r.capacity) : 0.0;
  arr.push_back({
    {"routeId", r.id},
    {"fromStationId", r.fromStationId},
    {"toStationId", r.toStationId},
    {"capacity", r.capacity},
    {"occupiedSeats", r.occupiedSeats},
    {"availableSeats", r.availableSeats()},
    {"occupancyPercent", pct}
  });
}
```

### Frontend Visualization (web/app.js)
```javascript
const cls = pct < 50 ? 'low' : pct < 80 ? 'medium' : 'high';
```
- **Green** (< 50%): Low occupancy
- **Orange** (50-80%): Moderate occupancy
- **Red** (> 80%): High occupancy (surge pricing active!)

Pure CSS bar chart — no chart library needed.

### Graph Analysis Insight
This maps the **state** of each graph edge. Combined with the route graph topology, you can see:
- Which segments are congested (red edges)
- Which stations need more outgoing routes (all edges from a station are red)
- Which alternate paths exist for congested routes

---

## 5. Fare Trend Predictor (Custom Feature)

### What it is
Simulates occupancy growth over N days and predicts when surge pricing will trigger. This is a **graph analysis** tool — it answers "what if?" about route usage.

### How It Works
```cpp
int simulated = route->occupiedSeats;
int step = std::max(1, (route->capacity - route->occupiedSeats) / days);

for (int d = 1; d <= days; d++) {
  Route r = *route;                              // copy the route
  r.occupiedSeats = std::min(r.capacity, simulated);
  double fare = pricing_.currentFare(r);          // check surge at each step
  simulated += step;
}
```

**Algorithm**: Linearly interpolates occupancy from current → full over the given days. At each step, checks if ownership crosses the 80% surge threshold.

### Example Output
```json
{
  "routeId": "R1",
  "days": 7,
  "trend": [
    {"day": 1, "projectedFare": 100.0, "occupancyPercent": 80.0},
    {"day": 2, "projectedFare": 130.0, "occupancyPercent": 84.0}
  ]
}
```
R1 base fare is $100. On day 2, occupancy crosses 80% → surge triggers → fare jumps to $130.

---

## 6. API Endpoints You Own

| Endpoint | What it returns | Algorithm behind it |
|----------|----------------|---------------------|
| `GET /api/routes/search?from=X&to=Y&mode=M` | Dijkstra path result | P3's algorithm, you serve it |
| `GET /api/mst?start=X` | Prim's MST result | P3's algorithm, you serve it |
| `GET /api/reports/profitability` | Revenue-per-route map | `ReportEngine` (yours) |
| `GET /api/occupancy-map` | Occupancy % per route | Direct graph analysis (yours) |
| `GET /api/fare-trend?routeId=X&days=N` | Multi-day projection | Trend predictor (yours) |
| `GET /api/fares/range?min=X&max=Y` | Bookings in fare range | Binary search (yours) |

---

## Common Viva Questions

**Q: What sorting algorithm does `std::sort` use?**
**Introsort** — starts with Quicksort, switches to Heapsort if recursion depth exceeds O(log N) (prevents O(N²) worst case), uses Insertion Sort for small sub-arrays (<16 elements). Worst case guaranteed O(N log N).

**Q: Why `lo + (hi - lo) / 2` instead of `(lo + hi) / 2`?**
Prevents integer overflow. Both formulas give the same mathematical result, but `(lo + hi)` can overflow a 32-bit signed integer when both values are large.

**Q: What does `{-1, -1}` mean in binary search?**
"No bookings found in this fare range." The API returns an empty array when it sees this sentinel value.

**Q: How does the fare trend predictor work?**
Linearly simulates occupancy growth from current level → full capacity over N days. At each day, checks if surge pricing would trigger. It's a **predictive model** — not real-time data — showing "what if bookings continue at this rate."

**Q: Why `std::map` for profitability instead of `unordered_map`?**
`std::map` sorts keys (route IDs) automatically. The report output is naturally ordered (R1, R2, R3...). `unordered_map` would need a separate sort step.

**Q: Explain the difference between `std::map` and `std::unordered_map`.**
`map` is a balanced BST (Red-Black Tree) — O(log N) operations, sorted keys. `unordered_map` is a hash table — O(1) average, unsorted. We chose `map` for sorted output without extra work.

**Q: How does the occupancy map color coding work?**
API returns `occupancyPercent` (0-100). Frontend maps: <50% → green (`.low`), 50-80% → orange (`.medium`), >80% → red (`.high`). Pure CSS.

**Q: How would you add a new report?**
1. Add logic function in `ReportEngine` (or new class)
2. Add REST endpoint in `api_server.cpp`
3. Add tab/section in `web/index.html`
4. Add fetch + render in `web/app.js`
5. Add unit test in `tests/`

**Q: What graph analysis does your occupancy map enable?**
It overlays route utilization onto the graph topology. You can see: which segments are congested, whether alternate paths exist, and which stations might need new routes. It turns raw data into actionable insight.

**Q: Why is fare range search useful?**
Passengers looking for budget options can say "show me all bookings under $150." Binary search finds the range instantly from the sorted fare list. Without it, we'd scan every booking.

**Q: Can you explain the 3-key sort comparator?**
```cpp
return a.routeId < b.routeId ?:        // primary key
       a.travelDate < b.travelDate ?:   // secondary key
       a.chargedFare < b.chargedFare;   // tertiary key
```
Short-circuit evaluation: compare first key. If equal, compare second. If equal, compare third. This is the standard pattern for multi-key sorting in C++.

**Q: How do your reports depend on the graph structure?**
Every report ultimately queries the graph. Occupancy iterates graph edges. Profitability sums bookings per route (edge). Fare trend simulates future state on edges. Even sorting/search works on bookings that reference route IDs. The graph is the central data structure that all analysis revolves around.
