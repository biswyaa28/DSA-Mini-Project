# Viva Prep — P4: Graph + Route Analysis + Reports

## Your Role
You handle the analysis layer — sorting bookings, searching by fare range, generating route profitability reports, computing the seat occupancy map, and predicting fare trends. You also own the API endpoints that serve this data.

## Key Files You Own

| File | Lines | What it does |
|------|-------|-------------|
| `src/sort_search.cpp` | 59 | Multi-level sort + binary search on fare |
| `src/report_engine.cpp` | 10 | Profitability by route |
| `src/api_server.cpp` (analysis endpoints) | ~100 | 5 analysis/report endpoints |

You also depend on:
- `include/models.hpp` — Route, Booking structs (P1's area)
- `include/rail_graph.hpp` — to access route data
- `web/app.js` — renders your analysis in the browser

---

## 1. Multi-Level Sorting (`sort_search.cpp`)

### What it does
Sorts bookings by three keys in priority order:
1. **Route ID** (primary)
2. **Travel Date** (secondary)
3. **Charged Fare** (tertiary)

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

### How `std::sort` works
- Uses **Introsort** — a hybrid of Quicksort, Heapsort, and Insertion Sort
- Best case: O(N log N). Worst case: O(N log N)
- This is an **in-place** sort (modifies the original vector)

### Why this sorting order?
For a railway report: group by route (all bookings for R1 together), then within each route sort by date (chronological), then by fare (cheapest first). Makes the data readable and useful for analysis.

---

## 2. Binary Search on Fare Range (`sort_search.cpp`)

### What it does
Given a SORTED list of bookings by fare, find all bookings whose fare falls within [minFare, maxFare].

### Implementation (Two Binary Searches)
```cpp
std::pair<int, int> SortingAndSearch::binarySearchFareRange(
    const std::vector<Booking>& sortedFares,
    double minFare, double maxFare) {

  // Binary Search 1: Find LOWER BOUND (first index where fare >= minFare)
  int lo = 0, hi = sortedFares.size() - 1, start = -1;
  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    if (sortedFares[mid].chargedFare >= minFare) {
      start = mid;      // candidate found
      hi = mid - 1;     // search left half for earlier match
    } else {
      lo = mid + 1;     // search right half
    }
  }

  if (start == -1) return {-1, -1};  // nothing found

  // Binary Search 2: Find UPPER BOUND (last index where fare <= maxFare)
  lo = start;
  hi = sortedFares.size() - 1;
  int end = -1;
  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    if (sortedFares[mid].chargedFare <= maxFare) {
      end = mid;        // candidate found
      lo = mid + 1;     // search right half for later match
    } else {
      hi = mid - 1;     // search left half
    }
  }

  return {start, end};  // inclusive range
}
```

### Why Two Binary Searches Instead of One?
- First search finds the **start** of the range (lower bound)
- Second search finds the **end** of the range (upper bound)
- Two O(log N) searches = still O(log N) total
- A single linear search after finding start would be O(N) worst case

### Complexity
- Sorting: O(N log N)
- Binary search: O(log N)
- **Total**: O(N log N + log N) = O(N log N)

Without binary search (linear scan): O(N). Binary search is **only** faster when N is large and you search many times on the same sorted data.

### Why Binary Search Requires Sorted Data
Binary search works by repeatedly comparing the middle element to the target and discarding half the array. This ONLY works if the array is sorted — otherwise `fare[mid] < target` gives no information about which half to search next.

---

## 3. Profitability Report (`report_engine.cpp`)

### Implementation (Only 10 Lines!)
```cpp
std::map<std::string, double> ReportEngine::profitabilityByRoute(
    const std::vector<Booking>& bookings) {
  std::map<std::string, double> report;  // automatically sorted by key!
  for (const auto& b : bookings) {
    report[b.routeId] += b.chargedFare;  // sum fares per route
  }
  return report;
}
```

### Using `std::map` — Know This
- `std::map` is a **balanced binary search tree** (usually Red-Black Tree)
- Keys are stored in sorted order automatically
- Each `report[b.routeId] += fare` is an O(log N) insert/update
- If `routeId` doesn't exist yet, `operator[]` creates it with value 0.0, then adds fare
- Total: O(N log N) for N bookings

### Result Example
```
{"R1": 450.50, "R2": 300.00, "R3": 650.00}
```

---

## 4. Seat Occupancy Map (Custom Feature)

### How It Works
```cpp
// In api_server.cpp:
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

### Frontend Rendering (in app.js)
```javascript
const cls = pct < 50 ? 'low' : pct < 80 ? 'medium' : 'high';
// Renders colored bar: green = <50%, orange = 50-80%, red = >80%
```

Visual bar chart using pure CSS — no chart library needed.

---

## 5. Fare Trend Predictor (Custom Feature)

### How It Works
```cpp
int simulated = route->occupiedSeats;
int step = std::max(1, (route->capacity - route->occupiedSeats) / days);

for (int d = 1; d <= days; d++) {
  Route r = *route;                     // copy the route
  r.occupiedSeats = std::min(r.capacity, simulated);  // simulate increasing occupancy
  double fare = pricing_.currentFare(r);  // check if surge would trigger
  simulated += step;
}
```

- **Simulates** occupancy growing linearly over N days
- At each step, checks if surge pricing (>80%) would activate
- Returns projected fare + occupancy for each day

### Example Output
```json
{
  "routeId": "R1",
  "days": 7,
  "trend": [
    {"day": 1, "projectedFare": 100.0, "projectedOccupancy": 40, "occupancyPercent": 80.0},
    {"day": 2, "projectedFare": 130.0, "projectedOccupancy": 42, "occupancyPercent": 84.0},
    // surge kicks in on day 2 when occupancy crosses 80%!
  ]
}
```

---

## 6. API Endpoints You Own

| Endpoint | What it returns | Used by |
|----------|----------------|---------|
| `GET /api/routes/search?from=X&to=Y&mode=M` | Path result from Dijkstra | P3's algorithm |
| `GET /api/mst?start=X` | MST result from Prim | P3's algorithm |
| `GET /api/reports/profitability` | Revenue per route | Your analysis |
| `GET /api/occupancy-map` | Occupancy % per route | Your analysis |
| `GET /api/fare-trend?routeId=X&days=N` | Multi-day fare projection | Your analysis |
| `GET /api/fares/range?min=X&max=Y` | Bookings within fare range | Your analysis |

---

## Common Viva Questions

**Q: What sorting algorithm does `std::sort` use?**
**Introsort** — starts with Quicksort, switches to Heapsort if recursion depth gets too deep, and uses Insertion Sort for small sub-arrays. Worst case is O(N log N) unlike raw Quicksort which can degrade to O(N²).

**Q: What is the mid calculation `lo + (hi - lo) / 2` and why not `(lo + hi) / 2`?**
It prevents **integer overflow**. If `lo` and `hi` are both large (e.g., 2 billion), their sum can overflow a 32-bit int. `lo + (hi - lo)/2` is mathematically equivalent but overflow-safe.

**Q: What does `{-1, -1}` mean in binary search return?**
Indicates "no bookings found in this fare range." The API checks for this and returns an empty results array.

**Q: How does the fare trend predictor work?**
It linearly simulates occupancy growth over the given days. At each day, it checks if surge pricing (>80% occupancy) would trigger, and returns the projected fare. It's a **predictive model** — not real data.

**Q: Why did you use `std::map` for profitability instead of `std::unordered_map`?**
`std::map` keeps routes in alphabetical order automatically. This makes the report output naturally organized. `std::unordered_map` would need a separate sort step.

**Q: What's the difference between `std::map` and `std::unordered_map`?**
| | `std::map` | `std::unordered_map` |
|---|---|---|
| Internal structure | Balanced BST (Red-Black Tree) | Hash table |
| Key ordering | Sorted ascending | Unsorted |
| Lookup | O(log N) | O(1) average |
| Insert | O(log N) | O(1) average |

**Q: How does the occupancy map color coding work?**
The API returns `occupancyPercent` as a raw number (0-100). The frontend CSS applies a color class: `< 50%` = green (`.low`), `50-80%` = orange (`.medium`), `> 80%` = red (`.high`).

**Q: How would you add a new type of report?**
1. Add the logic function in `ReportEngine` (or a new class)
2. Add a new endpoint in `api_server.cpp`
3. Add a new tab/section in `web/index.html`
4. Add the fetch + render function in `web/app.js`
5. Add a unit test in `tests/`
