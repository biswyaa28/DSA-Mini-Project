# Viva Preparation Notes — Algorithm & DSA Specialist

## Your Role
You implemented all the core Data Structures & Algorithms. This is the most important part of the viva — DSA is the heart of the project.

## Key Files You Own

| File | Key Algorithm | Lines |
|------|--------------|-------|
| `src/routing_engine.cpp` | Dijkstra (modified) | 89 |
| `src/mst_engine.cpp` | Prim's Algorithm | 47 |
| `src/sort_search.cpp` | Multi-level sort + Binary search | 59 |
| `src/pricing_engine.cpp` | Greedy surge pricing | 13 |
| `src/booking_engine.cpp` | Queue (waitlist) + Stack (refunds) | 119 |
| `include/models.hpp` | Data structures | 50 |

---

## 1. Dijkstra's Algorithm (`routing_engine.cpp`)

### Standard Dijkstra
- **Purpose**: Find shortest path in a weighted graph from source to destination.
- **Data Structures**: Distance map, previous-node map, visited set.
- **Complexity**: O(V²) with array-based min-find, O((V+E) log V) with priority queue.

### Your Implementation
```cpp
// Simplified version from your code:
while (true) {
  // Find unvisited node with smallest distance (linear scan = O(V))
  for (auto& pair : dist) {
    if (!visited.count(pair.first) && pair.second < minDist) {
      minDist = pair.second; current = pair.first;
    }
  }
  if (current.empty() || current == dst) break;
  visited.insert(current);
  for (Route& route : graph_.outgoing(current)) {
    double w = edgeWeight(route, mode);
    // Relax edge
    if (!dist.count(route.toStationId) || dist[current] + w < dist[route.toStationId]) {
      dist[route.toStationId] = dist[current] + w;
      prev[route.toStationId] = current;
      routeTaken[route.toStationId] = route.id;
    }
  }
}
```

### Modified Dijkstra for "Least Crowded" Mode
```cpp
double edgeWeight(const Route& route, const std::string& mode) {
  if (mode == "shortest") return route.distanceKm;
  // least_crowded: weight = distance * (1 + occupancyRatio)
  double occupancyRatio = route.occupiedSeats / route.capacity;
  return route.distanceKm * (1.0 + occupancyRatio);
}
```
- **Penalizes crowded routes**: A 100% full route doubles its effective distance.
- **No priority queue**: Uses linear scan O(V) to find minimum. Acceptable for small graphs (5 stations).

### Viva Questions
**Q: Why does your Dijkstra not use a priority queue?**
The graph is small (max 5 stations, 7 routes). Linear scan is simpler to implement and the O(V²) vs O((V+E) log V) difference is negligible for this size.

**Q: How does "least crowded" mode actually work?**
The edge weight becomes `distance × (1 + occupancy_ratio)`. A fully occupied route (ratio=1.0) has 2× the effective distance, discouraging the algorithm from choosing it.

**Q: What if there's no path?**
`dist.count(dst)` returns false, and we return an empty `PathResult`. The API then responds with 404.

---

## 2. Prim's Algorithm (`mst_engine.cpp`)

### Standard Prim's
- **Purpose**: Find Minimum Spanning Tree — the cheapest set of edges connecting all nodes.
- **Approach**: Greedy. Start from any node, repeatedly add the cheapest edge connecting the visited set to an unvisited node.

### Your Implementation
```cpp
std::unordered_set<std::string> inMst;
inMst.insert(startStation);

while (true) {
  double bestWeight = INF;
  for (const std::string& node : inMst) {
    for (const Route& route : graph_.outgoing(node)) {
      if (inMst.count(route.toStationId)) continue; // skip already connected
      if (route.distanceKm < bestWeight) {
        bestWeight = route.distanceKm;
        bestRouteId = route.id;
        bestDest = route.toStationId;
      }
    }
  }
  if (bestRouteId.empty()) break; // no more reachable nodes
  result.routeIds.push_back(bestRouteId);
  result.totalDistance += bestWeight;
  inMst.insert(bestDest);
}
```

### Viva Questions
**Q: Is this Prim's or Kruskal's?** Prim's. We grow from a starting node, adding the cheapest edge connecting visited-to-unvisited. Kruskal's sorts all edges globally and adds them one-by-one using Union-Find.

**Q: Complexity?** O(V² × E) in this implementation (for each node in MST, scan all edges from visited set). Can be optimized to O(E log V) with a priority queue — but fine for small graphs.

**Q: What does MST represent in railway context?** The minimum total track length needed to connect all stations so every station is reachable from every other station. Useful for cost planning.

---

## 3. Sorting & Binary Search (`sort_search.cpp`)

### Multi-Level Sorting
```cpp
std::sort(bookings.begin(), bookings.end(),
  [](const Booking& a, const Booking& b) {
    if (a.routeId != b.routeId) return a.routeId < b.routeId; // 1st key
    if (a.travelDate != b.travelDate) return a.travelDate < b.travelDate; // 2nd key
    return a.chargedFare < b.chargedFare; // 3rd key
  });
```

### Binary Search for Fare Range
```cpp
// Two binary searches: one for lower bound, one for upper bound
int lo = 0, hi = size-1, start = -1;
while (lo <= hi) { // Find first index with fare >= minFare
  int mid = lo + (hi - lo) / 2;
  if (sortedFares[mid].chargedFare >= minFare) { start = mid; hi = mid - 1; }
  else lo = mid + 1;
}
// Similar loop for upper bound (fare <= maxFare)
```

### Viva Questions
**Q: Why does binary search require sorted data?** It works by repeatedly dividing the search space in half. This only works if the array is sorted — otherwise the "less than / greater than" comparison has no meaning.

**Q: Complexity?** Sorting: O(N log N). Binary search: O(log N) — much faster than linear O(N).

**Q: What happens if no bookings are in the fare range?** Returns `{-1, -1}` indicating empty range, and the API shows 0 results.

---

## 4. Greedy Surge Pricing (`pricing_engine.cpp`)

```cpp
constexpr double kSurgeMultiplier = 1.3;

double currentFare(const Route& route) {
  if (route.isSurgePricing()) return route.baseFare * kSurgeMultiplier;
  return route.baseFare;
}
```

Where `isSurgePricing()` is:
```cpp
return occupiedSeats * 100 > capacity * 80;  // >80% occupancy → surge
```

### Why is this a Greedy Algorithm?
It makes the locally optimal decision (raise price now) without considering future consequences. A greedy algorithm takes the best immediate choice.

### Viva Questions
**Q: Why not use dynamic programming for pricing?** Surge pricing is a simple threshold rule. There's no need to optimize over a sequence of decisions — the fare depends only on current occupancy. DP would be overkill.

**Q: What's the 30% surge meant to simulate?** Real-world behavior: as demand increases, prices rise to reduce demand and increase supply (driver incentives in Uber, premium fares in Indian Railways Tatkal).

---

## 5. Queue (Waitlist) & Stack (Refunds) (`booking_engine.cpp`)

### VIP-First Waitlist Queue
```cpp
std::deque<Booking> waitlistVip_;     // VIP passengers
std::deque<Booking> waitlistRegular_;  // Regular passengers

void addToWaitlist(const Booking& booking) {
  if (booking.isVip) waitlistVip_.push_back(booking);
  else waitlistRegular_.push_back(booking);
}

bool promoteWaitlisted(const std::string& routeId) {
  // VIPs promoted first
  if (!waitlistVip_.empty()) {
    Booking booking = waitlistVip_.front();  // FIFO
    waitlistVip_.pop_front();
    // ... confirm seat, update status
  }
  // Then regular
}
```

**Why `std::deque` instead of `std::queue`?** Both support push_back/pop_front (FIFO). Deque is more flexible and the container adaptor `std::queue` defaults to deque internally anyway.

### Refund Stack (LIFO)
```cpp
std::vector<std::string> refundStack_;  // acts as stack

// When cancelling:
refundStack_.push_back(bookingId);  // push

// Getting last refund:
return refundStack_.back();  // peek (top)
```

### Viva Questions
**Q: Why is waitlist a queue (FIFO) but refunds a stack (LIFO)?** 
- **Queue**: First-come, first-served is fair — the person who waited longest gets the seat.
- **Stack**: Last-in, first-out for refunds mirrors real systems (the most recent cancellation is often the one the customer wants to check on).

**Q: What happens when a VIP cancels?**

```
cancelBooking(bookingId):
  status → Cancelled
  push to refundStack_
  if (previousStatus == Confirmed):
    decrement occupiedSeats on the route
    promoteWaitlisted(routeId)  // auto-promote next in line!
```

The passenger immediately behind them (VIP or regular) gets auto-promoted from waitlist to confirmed. This is the **auto-promotion** feature.

**Q: How do you detect surge pricing in the Route model?**
```cpp
bool isSurgePricing() const {
  if (capacity <= 0) return false;
  return occupiedSeats * 100 > capacity * 80;  // avoids floating point
}
```
Using integer math avoids floating point precision issues. The expression checks: "are occupied seats more than 80% of capacity?"

---

## Summary of DSA Concepts Used

| Concept | Where | Complexity |
|---------|-------|------------|
| Graph (Adjacency List) | `RailGraph` — vector of edges per station | O(E) traversal |
| Dijkstra | Shortest/least-crowded path | O(V²) or O((V+E) log V) |
| Prim's MST | Minimum cost network | O(V² × E) |
| Queue (FIFO) | VIP/regular waitlists | O(1) push/pop |
| Stack (LIFO) | Refund history | O(1) push/pop/top |
| Sorting | Multi-level sort (route → date → fare) | O(N log N) |
| Binary Search | Fare range search | O(log N) |
| Greedy Algorithm | Surge pricing at >80% | O(1) |
