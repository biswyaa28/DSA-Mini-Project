# Viva Prep — P1: Station & Train Records (Linked List / Arrays)

## Your Role
You manage all the data structures that store stations, routes, and bookings — basically the "database" layer using arrays (vectors) and linear search.

## Key Files You Own

| File | Lines | What it does |
|------|-------|-------------|
| `include/models.hpp` | 50 | Defines `Station`, `Route`, `Booking` structs |
| `include/station_directory.hpp` + `src/station_directory.cpp` | 60 | Array-based station storage |
| `include/rail_graph.hpp` + `src/rail_graph.cpp` | 60 | Array-based route storage (the graph edges) |
| `src/storage_manager.cpp` | 52 | Save/load bookings to JSON using nlohmann/json |

---

## 1. The Data Structures — Know These Cold

### Station
```cpp
struct Station {
  std::string id;    // e.g. "S1", "S2"
  std::string name;  // e.g. "Central", "West"
};
```

### Route
```cpp
struct Route {
  std::string id;
  std::string fromStationId;  // source station
  std::string toStationId;    // destination station
  int distanceKm;             // edge weight
  double baseFare;            // base price
  int capacity;               // total seats
  int occupiedSeats;          // currently booked

  // helper methods (inline in header):
  int availableSeats() const;
  bool isSurgePricing() const;
};
```

### Booking
```cpp
enum class BookingStatus { Confirmed, Waitlisted, Cancelled };

struct Booking {
  std::string id;          // auto-generated: "B1", "B2"...
  std::string routeId;
  std::string passengerName;
  bool isVip;
  BookingStatus status;
  double chargedFare;
  std::string travelDate;
};
```

### Why struct not class?
Plain data containers with all public fields. No encapsulation needed — getters/setters would be pointless boilerplate.

### Why `isSurgePricing()` uses integer math?
```cpp
return occupiedSeats * 100 > capacity * 80;
```
Avoids floating-point division. Checks: "are more than 80% of seats occupied?"

---

## 2. StationDirectory — Array-Based Storage

```cpp
class StationDirectory {
  std::vector<Station> stations_;  // dynamic array

  bool addStation(const Station& s);        // returns false if duplicate ID
  bool hasStation(const std::string& id);
  const Station* findStation(const std::string& id);  // linear search
  std::vector<Station> allStations();
};
```

### Key Operations
| Operation | How it works | Complexity |
|-----------|-------------|------------|
| Add | `push_back` into vector | O(1) amortized |
| Find by ID | Linear scan through vector | O(N) |
| Duplicate check | Scan before insert | O(N) |
| List all | Return copy of vector | O(N) |

### Why vector instead of unordered_map?
- Only 5 stations in our seed data. O(N) linear scan with N=5 is instant
- Simpler code, no hash function to worry about
- If we had 10,000 stations, we'd switch to `std::unordered_map` for O(1) lookups

---

## 3. RailGraph — Edge List (Array of Routes)

```cpp
class RailGraph {
  std::vector<Route> routes_;  // flat list of all route edges

  bool addRoute(const Route& r);
  bool hasRoute(const std::string& id);
  const Route* findRoute(const std::string& id);  // linear search by ID
  std::vector<Route> outgoing(const std::string& stationId);
  std::vector<Route> allRoutes();
  bool updateOccupiedSeats(const std::string& routeId, int seats);
};
```

### How "outgoing" works (the adjacency query)
```cpp
std::vector<Route> RailGraph::outgoing(const std::string& stationId) const {
  std::vector<Route> result;
  for (const auto& r : routes_) {
    if (r.fromStationId == stationId)
      result.push_back(r);
  }
  return result;
}
```
This is **not a true adjacency list** (which would be `map<stationId, list<edges>>`). It's an edge list that scans all routes every time. O(E) per query.

### Why edge list instead of adjacency list?
| | Edge List (ours) | Adjacency List |
|---|---|---|
| Storage | Single vector | Map of vectors |
| `outgoing()` | O(E) scan | O(1) lookup |
| Add edge | O(1) | O(1) |
| Simplicity | Very simple | Requires map |

For 7 routes, O(E) = O(7) is nothing. Simpler wins.

### Find vs Pointer Return
```cpp
const Route* findRoute(const std::string& id) const;
```
Returns `nullptr` if not found instead of throwing. Callers check for null. This is the "optional return" pattern (before C++17 had `std::optional`).

---

## 4. Seed Data — 5 Stations, 7 Routes

```
S1(Central) ──R1──▶ S2(West) ──R2──▶ S3(North)
S1 ──R3─────────────────────────────▶ S3
S3 ──R4──▶ S4(East)
S2 ──R5─────────────────────────────▶ S4
S1 ──R6──▶ S5(South) ──R7──▶ S4
```

All 5 stations are connected. No isolated nodes.

---

## 5. StorageManager — JSON Persistence

```cpp
class StorageManager {
  bool saveBookings(const std::vector<Booking>&, const std::string& filepath);
  std::vector<Booking> loadBookings(const std::string& filepath);
};
```

- Uses `nlohmann/json` for serialization
- `NLOHMANN_JSON_SERIALIZE_ENUM` macro maps `BookingStatus` enum to strings
- Bookings saved to `data/bookings.json` at runtime
- If file doesn't exist, returns empty vector (graceful startup)

---

## Common Viva Questions

**Q: What STL container did you use and why?**
`std::vector` — dynamic array. Contiguous memory cache-friendly, O(1) amortized push_back, and we don't need insert/delete in the middle.

**Q: Why didn't you use a linked list?**
`std::vector` outperforms `std::list` in almost every scenario for small-to-medium sized data because of cache locality. Linked lists would only be better if we frequently inserted/deleted in the middle — which this project doesn't do.

**Q: The question says "linked list/arrays" — which did you actually use?**
Arrays (vectors). Stations and routes are stored in `std::vector` which is a dynamic array internally. The "linked list" concept is more relevant in the waitlist queue (P2's area).

**Q: How would you scale this to 10,000 stations?**
1. Replace `StationDirectory::findStation()` linear scan with `std::unordered_map<std::string, Station>` for O(1) lookups
2. Replace `RailGraph::outgoing()` linear scan with a prebuilt `std::unordered_map<std::string, std::vector<Route>>` adjacency list
3. Dijkstra's O(V²) scan would need a priority queue (`std::priority_queue`) for O((V+E) log V)

**Q: How does `findRoute` return a pointer to data inside the vector?**
```cpp
const Route* RailGraph::findRoute(const std::string& id) const {
  for (const auto& r : routes_) {
    if (r.id == id) return &r;  // pointer to element inside vector
  }
  return nullptr;
}
```
This is safe as long as the vector isn't modified while the pointer is in use. Vector push_back can reallocate, invalidating pointers.

**Q: What happens if `data/` directory doesn't exist?**
`StorageManager::loadBookings()` tries to open the file — if it fails, it returns an empty vector. The server starts fine with no saved data.

**Q: Explain the JSON serialization macros**
```cpp
NLOHMANN_JSON_SERIALIZE_ENUM(BookingStatus, {
    {BookingStatus::Confirmed, "Confirmed"},
    {BookingStatus::Waitlisted, "Waitlisted"},
    {BookingStatus::Cancelled, "Cancelled"},
})
```
This macro tells nlohmann/json how to convert the C++ enum to/from JSON strings. Without this, the library wouldn't know how to serialize `BookingStatus`.
