# P1: Station & Train Records — Linked Lists / Arrays

## Your Role
You are responsible for **station and train records** using **Linked Lists or Arrays**. You own all data storage — the models, the array-based station/route containers, and JSON persistence. When the viva asks "what container and why," that's your question.

## Files You Own

| File | Lines | What it does |
|------|-------|--------------|
| `include/models.hpp` | 50 | Defines `Station`, `Route`, `Booking` structs — the record types |
| `include/station_directory.hpp` + `src/station_directory.cpp` | ~50 | Array-based station record storage (add, find, list, duplicate detection) |
| `include/rail_graph.hpp` + `src/rail_graph.cpp` | ~70 | Array-based route record storage — the graph's edge list |
| `include/storage_manager.hpp` + `src/storage_manager.cpp` | ~55 | JSON persistence for booking records using nlohmann/json |

---

## 1. The Record Types (Data Models)

### Station Record
```cpp
struct Station {
  std::string id;    // primary key: "S1", "S2"
  std::string name;  // "Central", "West"
};
```
Simple key-value record. `id` is the unique identifier used as a graph node label.

### Route Record (Edge Record)
```cpp
struct Route {
  std::string id;              // "R1", "R2"
  std::string fromStationId;   // source node
  std::string toStationId;     // destination node
  int distanceKm;              // edge weight for Dijkstra/Prim
  double baseFare;             // base price
  int capacity;                // total seats
  int occupiedSeats;           // currently booked

  int availableSeats() const;  // capacity - occupiedSeats
  bool isSurgePricing() const; // true if >80% occupied
};
```

### Booking Record
```cpp
enum class BookingStatus { Confirmed, Waitlisted, Cancelled };

struct Booking {
  std::string id;           // "B1", "B2" — auto-incremented
  std::string routeId;      // FK to Route
  std::string passengerName;
  bool isVip;
  BookingStatus status;
  double chargedFare;       // what the passenger actually paid
  std::string travelDate;   // "2025-01-15"
};
```

### Why `struct` not `class`?
Plain data records — all fields public. No encapsulation needed. Getters/setters would be pointless boilerplate.

### `isSurgePricing()` — Integer Math (No Floats)
```cpp
bool Route::isSurgePricing() const {
  if (capacity <= 0) return false;
  return occupiedSeats * 100 > capacity * 80;
}
```
`occupiedSeats * 100` avoids floating-point division. Checks if >80% seats occupied. This is used by P3's pricing engine.

---

## 2. StationDirectory — Array-Based Station Records

```cpp
class StationDirectory {
  std::vector<Station> stations_;

  bool addStation(const Station& s);
  bool hasStation(const std::string& id);
  const Station* findStation(const std::string& id);  // linear search → ptr or nullptr
  std::vector<Station> allStations();
};
```

### Complexity

| Operation | Mechanism | Complexity |
|-----------|-----------|------------|
| Add station | `push_back` | O(1) amortized |
| Find by ID | Linear scan | O(N) |
| Duplicate check | Scan before insert | O(N) |
| List all | Return copy | O(N) |

### Why Array (vector) over Linked List?

| Property | `std::vector` (Array) | `std::list` (Linked List) |
|----------|----------------------|--------------------------|
| Memory | Contiguous block | Fragmented nodes |
| Random access | O(1) — `vec[3]` | O(N) — must traverse |
| Insert at end | O(1) amortized | O(1) |
| Insert in middle | O(N) — shift elements | O(1) — relink pointers |
| Cache performance | Excellent (prefetching) | Poor (scattered addresses) |
| Memory overhead | 0 per element | 2 pointers per node |

**Why vector won here:**
- We only append records (no middle insertions/deletions)
- Contiguous memory = CPU cache friendly = faster iteration
- For N=5 stations, both are instant, but vector is the better default

**When would linked list be better?**
- Frequent insertions/deletions in the middle of the list
- Very large records where shift cost is high
- Our project has neither — vector is correct.

### Why not `unordered_map` for O(1) lookups?
For 5 stations, O(N) with N=5 is ~5 integer comparisons. A hash function has computation overhead. At 10,000+ stations, `unordered_map` would be the right choice.

---

## 3. RailGraph — Array-Based Route Records (Edge List)

```cpp
class RailGraph {
  std::vector<Route> routes_;

  bool addRoute(const Route& r);
  bool hasRoute(const std::string& id);
  const Route* findRoute(const std::string& id);  // linear scan by ID
  std::vector<Route> outgoing(const std::string& stationId);  // edges from a node
  std::vector<Route> allRoutes();
  bool updateOccupiedSeats(const std::string& routeId, int newValue);
};
```

### How `outgoing()` Works
```cpp
std::vector<Route> RailGraph::outgoing(const std::string& stationId) const {
  std::vector<Route> result;
  for (const auto& r : routes_) {
    if (r.fromStationId == stationId) result.push_back(r);
  }
  return result;
}
```
This is an **edge list** — a flat array of all graph edges. Every adjacency query scans ALL E edges.

### Edge List vs True Adjacency List

| Property | Edge List (ours, O(E)) | Adjacency List (O(1)) |
|----------|----------------------|----------------------|
| Storage | Single `vector<Route>` | `map<stationId, vector<Route>>` |
| `outgoing()` | O(E) scan every edge | O(1) map lookup |
| Add route | O(1) push_back | O(1) push to map entry |
| Memory pattern | Contiguous, cache-friendly | Pointers, fragmented |

**Why edge list for 7 routes?** O(E) = O(7). Faster to scan 7 structs than build/maintain a map. Simplicity wins.

### Pointer-to-Element Return
```cpp
const Route* RailGraph::findRoute(const std::string& id) const {
  for (const auto& r : routes_) {
    if (r.id == id) return &r;
  }
  return nullptr;
}
```
Returns `nullptr` for "not found" instead of throwing. **Safety**: pointer is valid only while vector isn't modified (push_back may reallocate).

---

## 4. StorageManager — Record Persistence

```cpp
struct StorageManager {
  static bool saveBookings(const std::vector<Booking>&, const std::string& path);
  static std::vector<Booking> loadBookings(const std::string& path);
};
```

- `nlohmann/json` serializes Booking records ↔ JSON
- `NLOHMANN_JSON_SERIALIZE_ENUM` maps `BookingStatus` enum ↔ strings
- Saved to `data/bookings.json`
- Missing file → returns empty vector (graceful startup)

---

## 5. Seed Data Records (MEMORIZE THIS)

```
S1(Central) ──R1(10km,$100)──▶ S2(West) ──R2(15km,$150)──▶ S3(North)
S1 ──────────────────R3(40km,$200)─────────────────────────▶ S3
S3 ──────────────────R4(20km,$120)─────────────────────────▶ S4(East)
S2 ──────────────────R5(25km,$180)─────────────────────────▶ S4
S1 ──R6(30km,$250)──▶ S5(South) ──R7(10km,$90)──▶ S4
```

**5 stations (S1-S5), 7 routes (R1-R7).** All connected, no isolated nodes. This is the input to every algorithm (Dijkstra, Prim's, reports).

---

## Common Viva Questions

**Q: What STL container did you use and why?**
`std::vector` — dynamic array. Contiguous memory is cache-friendly, O(1) amortized push_back, and we don't need insert/delete in the middle.

**Q: Why not linked list?**
`std::vector` outperforms `std::list` for our access patterns because of cache locality. Linked list only wins with frequent middle insertions — our project doesn't do that.

**Q: Your role says "Linked Lists or Arrays" — which did you actually implement?**
Arrays (vectors). Stations and routes are stored in `std::vector`, a dynamic array. The linked list concept is used by P2's waitlist queue (`std::deque`).

**Q: How would you scale to 10,000 stations?**
1. Station lookups: `unordered_map<string, Station>` (O(N) → O(1))
2. Graph outgoing: Prebuilt adjacency list `unordered_map<string, vector<Route>>` (O(E) → O(1))
3. Dijkstra: Priority queue for O((V+E) log V) instead of O(V²)

**Q: How is returning a pointer to vector data safe?**
`&r` points to an element inside the vector. Valid only while vector isn't modified. In our server, routes are seeded once and never changed, so it's safe.

**Q: What happens if `data/bookings.json` is missing?**
`loadBookings()` returns an empty vector. Server starts gracefully with no saved data.

**Q: Explain the JSON serialization.**
```cpp
NLOHMANN_JSON_SERIALIZE_ENUM(BookingStatus, {
    {BookingStatus::Confirmed, "Confirmed"},
    {BookingStatus::Waitlisted, "Waitlisted"},
    {BookingStatus::Cancelled, "Cancelled"},
})
```
Instructs nlohmann/json how to convert `BookingStatus` enum ↔ readable JSON strings.

**Q: What's the difference between array and linked list memory layouts?**
Array: contiguous block — all elements next to each other in RAM. Linked list: nodes scattered across RAM, each holding a pointer to the next. Arrays win for iteration (CPU prefetches adjacent data).

**Q: Why does `updateOccupiedSeats` return bool?**
It validates bounds: `if (newValue < 0 || newValue > route.capacity) return false`. Prevents seat count from going negative or exceeding capacity.

**Q: If you were to redesign for performance, what would you change?**
Replace the edge list with a true adjacency list (`unordered_map<string, vector<Route*>>`) so Dijkstra's neighbor iteration is O(degree) instead of O(E). Replace linear station search with `unordered_map`.
