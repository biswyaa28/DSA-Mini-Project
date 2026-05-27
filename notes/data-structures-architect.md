# Viva Preparation Notes — Data Structures & System Design

## Your Role
You designed the core data structures and the graph-based railway network model that everything else depends on.

## Key Files You Own

| File | Purpose | Lines |
|------|---------|-------|
| `include/models.hpp` | All data structures (Station, Route, Booking) | 50 |
| `include/rail_graph.hpp` + `src/rail_graph.cpp` | Graph representation | ~60 total |
| `include/station_directory.hpp` + `src/station_directory.cpp` | Station lookup | ~60 total |
| `include/api_server.hpp` | System composition (how all modules connect) | 36 |
| `src/main.cpp` | Application entry point | ~10 |

---

## 1. Data Structures (`models.hpp`)

### Station
```cpp
struct Station {
  std::string id;    // e.g. "S1", "S2"
  std::string name;  // e.g. "Central", "West"
};
```
Simple POD. Stations are uniquely identified by `id`.

### Route
```cpp
struct Route {
  std::string id;              // e.g. "R1", "R2"
  std::string fromStationId;   // Source station
  std::string toStationId;     // Destination station
  int distanceKm;              // Edge weight for shortest-path
  double baseFare;             // Base price (before surge)
  int capacity;                // Total seats
  int occupiedSeats;           // Currently booked seats

  int availableSeats() const;  // capacity - occupiedSeats (min 0)
  bool isSurgePricing() const; // occupiedSeats > 80% of capacity
};
```

**Design decisions:**
- **Directed edge**: `fromStationId → toStationId`. Routes are one-way.
- **Distance as int**: Straightforward for comparison and sum.
- **Fare as double**: Needed for fractional values like surge multiplier (1.3).
- **`isSurgePricing()` uses integer math**: `occupiedSeats * 100 > capacity * 80` — avoids floating point division.

### Booking
```cpp
enum class BookingStatus { Confirmed, Waitlisted, Cancelled };

struct Booking {
  std::string id;              // "B1", "B2", etc. (auto-incremented)
  std::string routeId;         // Which route
  std::string passengerName;
  bool isVip;                  // VIP gets priority waitlist promotion
  BookingStatus status;
  double chargedFare;          // Fare at time of booking (may include surge)
  std::string travelDate;      // Optional travel date
};
```

**Why enum class?** Scoped enum prevents naming conflicts (`BookingStatus::Confirmed` not just `Confirmed`).

---

## 2. RailGraph (`rail_graph.hpp` / `.cpp`)

### Graph Representation: Adjacency List (Edge List Variation)

```cpp
class RailGraph {
  std::vector<Route> routes_;  // All routes stored in a flat vector
public:
  bool addRoute(const Route& route);
  bool hasRoute(const std::string& routeId) const;
  const Route* findRoute(const std::string& routeId) const;
  std::vector<Route> allRoutes() const;
  std::vector<Route> outgoing(const std::string& stationId) const;
  std::vector<Route> outgoingRoutes(const std::string& stationId) const;
  bool updateOccupiedSeats(const std::string& routeId, int seats);
};
```

**This is NOT a classic adjacency list** (`map<station, list<edge>>`). It's a flat edge list that filters by `fromStationId` on demand.

```cpp
std::vector<Route> RailGraph::outgoing(const std::string& stationId) const {
  std::vector<Route> result;
  for (const auto& r : routes_) {
    if (r.fromStationId == stationId) result.push_back(r);
  }
  return result;  // O(E) each call
}
```

### Design Decision: Edge List vs True Adjacency List

| Aspect | Edge List (chosen) | Adjacency List |
|--------|-------------------|----------------|
| Storage | Flat vector | `unordered_map<station, vector<edge>>` |
| `outgoing()` | O(E) scan | O(1) lookup |
| Add route | O(1) push | O(1) push |
| Memory | Minimal | More (map overhead) |
| Complexity | Simple | More complex |

For 5 stations and 7 routes, O(E) is negligible. The simpler design wins.

---

## 3. StationDirectory

```cpp
class StationDirectory {
  std::vector<Station> stations_;
public:
  bool addStation(const Station& s);
  bool hasStation(const std::string& id) const;
  const Station* findStation(const std::string& id) const;
  std::vector<Station> allStations() const;
};
```

**Duplicate prevention**: `addStation` returns false if station ID already exists.

**Why not `std::map`?** Similar to RailGraph — linear scan in a vector is fine for 5 items. A map would be correct but unnecessarily complex for this scale.

---

## 4. System Architecture (How It All Connects)

### Object Composition (from `api_server.hpp`)

```cpp
class ApiServer {
  StationDirectory stations_;  // Station storage
  RailGraph graph_;            // Route graph
  PricingEngine pricing_;      // Fare calculation
  BookingEngine booking_;      // Booking + waitlist + refund
  RoutingEngine routing_;      // Dijkstra pathfinding
  MSTEngine mst_;              // Prim's MST
  StorageManager storage_;     // JSON persistence
  httplib::Server svr_;        // HTTP server
};
```

### Dependency Chain
```
BookingEngine ──uses──▶ RailGraph (to check/update seat occupancy)
RoutingEngine ──uses──▶ RailGraph (to traverse edges)
MSTEngine      ──uses──▶ RailGraph (to traverse edges)
ApiServer      ──uses──▶ All of the above
                ──uses──▶ PricingEngine (calculate fare at booking time)
                ──uses──▶ SortingAndSearch (fare range queries)
                ──uses──▶ ReportEngine (profitability reports)
```

---

## 5. Seed Data (Demo-Ready)

```cpp
void ApiServer::seedSampleData() {
  // 5 stations forming a graph
  // S1(Central) ──R1── S2(West) ──R2── S3(North)
  // S1 ──R3─────────────────────────▶ S3
  // S3 ──R4── S4(East)
  // S2 ──R5─────────────────────────▶ S4
  // S1 ──R6── S5(South) ──R7── S4
}
```

This creates a connected graph with exactly one shortest path between each pair (no ties to break).

---

## Common Viva Questions

**Q: Why struct instead of class for models?** Structs have public members by default. These are plain data containers with no encapsulation needed — getters/setters would be pointless boilerplate.

**Q: Why is `findRoute` returning a pointer instead of a reference?** Nullable return — if the route isn't found, we return `nullptr` instead of throwing an exception. Callers check for null. This is a C++ pattern for "optional" return before C++17's `std::optional`.

**Q: What if there are 10,000 stations?** The O(E) scans would become slow. We'd upgrade `StationDirectory` to `std::unordered_map` and `RailGraph::outgoing()` to a pre-built adjacency list. The rest of the code (Dijkstra, Prim) would work unchanged.

**Q: Why is BookingEngine separate from PricingEngine?** Single Responsibility Principle. Booking handles seat allocation and waitlist management. Pricing calculates fares. They could change independently (e.g., different surge formulas without touching booking logic).

**Q: How would you add bidirectional routes?** Add a `bidirectional` flag and automatically create two directed edges in `addRoute()`. Or just add two routes manually (R1: S1→S2, R1-rev: S2→S1).

**Q: Explain the data flow for "Book a seat"**
1. Browser sends POST /api/bookings with JSON body
2. ApiServer parses JSON, finds the Route
3. PricingEngine calculates fare (base or surge)
4. BookingEngine tries to confirm seat:
   - If capacity available → status = Confirmed, increment occupiedSeats
   - If full → status = Waitlisted, add to VIP/regular queue
5. JSON response sent back to browser
6. Browser refreshes bookings table

**Q: How does auto-promotion work when a seat frees up?**
1. User cancels → booking status → Cancelled
2. If it was Confirmed: decrement occupiedSeats on route
3. `promoteWaitlisted(routeId)` is called
4. First checks VIP queue (front), then regular queue
5. First person in queue gets seat → status → Confirmed
6. This is the exact same mechanism as pushing from waitlist → confirmed
