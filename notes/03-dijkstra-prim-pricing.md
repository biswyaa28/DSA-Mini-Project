# P3: Dijkstra's Algorithm + Prim's MST + Dynamic Pricing

## Your Role
You are responsible for the **three core algorithms** — Dijkstra's shortest path (two modes), Prim's Minimum Spanning Tree, and the greedy dynamic surge pricing logic. This is the algorithmic heart of the project.

## Files You Own

| File | Algorithm | Lines |
|------|-----------|-------|
| `src/routing_engine.cpp` | Dijkstra (2 modes: shortest, least-crowded) | 89 |
| `src/mst_engine.cpp` | Prim's Algorithm | 47 |
| `src/pricing_engine.cpp` | Greedy surge pricing | 13 |

You also depend on:
- `include/models.hpp` — `Route` struct with `distanceKm`, `capacity`, `occupiedSeats` (P1)
- `include/rail_graph.hpp` — `outgoing()` to get neighbor edges from a station (P1)

---

## 1. Dijkstra's Algorithm

### Pseudocode
```
dist[source] = 0
for each node: dist[node] = INF
visited = empty set

loop:
  current = unvisited node with smallest dist
  if current == destination: break
  visited.add(current)
  for each neighbor of current:
    newDist = dist[current] + edgeWeight(current, neighbor)
    if newDist < dist[neighbor]:
      dist[neighbor] = newDist
      prev[neighbor] = current
```

### Your Implementation (89 lines)
```cpp
PathResult RoutingEngine::findPath(const std::string& src,
                                    const std::string& dst,
                                    const std::string& mode) const {
  PathResult result;
  if (src == dst) return result;  // same station → empty path, 0 distance

  const double INF = std::numeric_limits<double>::max();
  std::unordered_map<std::string, double> dist;
  std::unordered_map<std::string, std::string> prev;
  std::unordered_map<std::string, std::string> routeTaken;  // which route was used
  std::unordered_set<std::string> visited;

  dist[src] = 0.0;

  while (true) {
    // O(V) linear scan to find unvisited node with minimum distance
    std::string current;
    double minDist = INF;
    for (const auto& pair : dist) {
      if (!visited.count(pair.first) && pair.second < minDist) {
        minDist = pair.second;
        current = pair.first;
      }
    }
    if (current.empty() || current == dst) break;
    visited.insert(current);

    // Relax all outgoing edges from current
    for (const Route& route : graph_.outgoing(current)) {
      double w = edgeWeight(route, mode);  // KEY: weight depends on mode
      double newDist = dist[current] + w;
      if (!dist.count(route.toStationId) || newDist < dist[route.toStationId]) {
        dist[route.toStationId] = newDist;
        prev[route.toStationId] = current;
        routeTaken[route.toStationId] = route.id;
      }
    }
  }

  // No path exists
  if (!dist.count(dst)) return PathResult{};

  // Reconstruct path (reverse from destination back to source)
  std::vector<std::string> path;
  std::string cur = dst;
  while (cur != src) {
    path.push_back(routeTaken[cur]);
    cur = prev[cur];
  }
  result.routeIds.assign(path.rbegin(), path.rend());  // reverse to get source→dest order
  result.totalDistance = dist[dst];
  return result;
}
```

### Edge Weight Function — The Key Innovation
```cpp
double RoutingEngine::edgeWeight(const Route& route,
                                  const std::string& mode) const {
  if (mode == "shortest") {
    return static_cast<double>(route.distanceKm);
  }
  // "least_crowded": penalize full routes
  double occupancyRatio = route.occupiedSeats / route.capacity;
  return route.distanceKm * (1.0 + occupancyRatio);
}
```

| Mode | Formula | When to use |
|------|---------|-------------|
| `shortest` | Raw `distanceKm` | Pure path length optimization |
| `least_crowded` | `distance × (1 + occupancyRatio)` | Avoid crowded trains |

**Example**: R1 is 10km at 100% occupancy → weight = 10 × (1 + 1.0) = **20**. R3 is 40km at 10% occupancy → weight = 40 × (1 + 0.1) = **44**. Dijkstra picks R1 path even though it involves a transfer, because the direct route is too crowded.

### PathResult Struct
```cpp
struct PathResult {
  std::vector<std::string> routeIds;  // ordered list of routes from source→dest
  double totalDistance;                // sum of distances
};
```
Clean return type — no output parameters, no exceptions.

---

## 2. Prim's Algorithm (Minimum Spanning Tree)

### Pseudocode
```
visited = {startNode}
while (edges exist from visited set to unvisited nodes):
    pick the cheapest such edge
    add the new node to visited set
```

### Your Implementation (47 lines)
```cpp
MstResult MSTEngine::buildMST(const std::string& startStation) const {
  MstResult result;
  std::unordered_set<std::string> inMst;
  inMst.insert(startStation);

  while (true) {
    double bestWeight = INF;
    std::string bestRouteId, bestDest;

    // Nested loop: for every visited node, scan all outgoing edges
    for (const std::string& node : inMst) {
      for (const Route& route : graph_.outgoing(node)) {
        if (inMst.count(route.toStationId)) continue;  // skip if already in MST
        double w = static_cast<double>(route.distanceKm);
        if (w < bestWeight) {
          bestWeight = w;
          bestRouteId = route.id;
          bestDest = route.toStationId;
        }
      }
    }

    if (bestRouteId.empty()) break;  // no more reachable nodes

    result.routeIds.push_back(bestRouteId);
    result.totalDistance += bestWeight;
    inMst.insert(bestDest);
  }
  return result;
}
```

### MstResult Struct
```cpp
struct MstResult {
  std::vector<std::string> routeIds;  // edges in the MST
  double totalDistance;                // sum of all edge distances in MST
};
```

### How Cycles Are Avoided
The `inMst` set tracks all nodes already in the MST. When scanning edges, `if (inMst.count(route.toStationId)) continue;` skips any edge whose destination is already in the tree. This guarantees no cycles.

### Prim's vs Kruskal's — Know This!

| | Prim's (ours) | Kruskal's |
|---|---|---|
| Approach | Grow from seed node, add cheapest connecting edge | Sort ALL edges, add cheapest that doesn't create cycle |
| Data structure | Visited set + edge scan | Union-Find (Disjoint Set) |
| Start node | Required | Any |
| Best for | Dense graphs | Sparse graphs |
| Cycle detection | Check destination in visited set | Union-Find.find(parent) |

**Why Prim's here?** The railway "minimum cost network" question naturally starts from a specific station. Prim's grows outward — intuitively like building railway lines from a central hub.

### MST on Seed Data
For 5 stations, the MST contains exactly 4 edges (V-1). Starting from S1:
- Cheapest edge from {S1} → R1 (10km, to S2)
- Cheapest from {S1, S2} → R2 (15km, to S3) or R5 (25km, to S4)
- Continues until all 5 stations are connected

---

## 3. Greedy Surge Pricing

### Implementation
```cpp
constexpr double kSurgeMultiplier = 1.3;  // 30% surge

double PricingEngine::currentFare(const Route& route) const {
  if (route.isSurgePricing()) {
    return route.baseFare * kSurgeMultiplier;
  }
  return route.baseFare;
}
```

Called from `models.hpp`:
```cpp
bool Route::isSurgePricing() const {
  if (capacity <= 0) return false;
  return occupiedSeats * 100 > capacity * 80;
}
```

### Why This is Greedy
A **greedy algorithm** makes the locally optimal choice at each step without considering future consequences. Here: "if the train is >80% full right now, raise the fare." It ignores:
- Future demand trends
- Historical pricing patterns
- Overall revenue optimization
- Competitor pricing

**Greedy works here** because pricing is a threshold decision, not a multi-step optimization. Simple 1.3× surge at 80% threshold is exactly how Uber and Indian Railways' Tatkal schemes work.

---

## 4. Complexity Analysis

| Algorithm | Your Complexity | Optimal Complexity | Why Not Optimize? |
|-----------|----------------|-------------------|-------------------|
| Dijkstra | O(V²) — linear min-find | O((V+E) log V) with priority queue | V=5, E=7. ~25 vs ~30 ops |
| Prim's | O(V² × E) — nested loops | O(E log V) with priority queue | Same reason |
| Surge Pricing | O(1) | O(1) | Optimal already |

**Optimization path for Dijkstra**: Replace the linear min-find loop with `std::priority_queue<std::pair<double, string>>`. Each iteration pops the min in O(log V). Total O((V+E) log V).

---

## Common Viva Questions

**Q: Explain the two Dijkstra modes.**
The `edgeWeight()` function returns different values based on the mode parameter. `shortest` uses raw distance. `least_crowded` uses `distance × (1 + occupancyRatio)` which makes full trains look "further away" to the algorithm. Everything else — the loop, the relaxation, the path reconstruction — is identical.

**Q: What's the complexity of your Dijkstra?**
O(V²) because we scan all distances to find the minimum each iteration (no priority queue). With V=5, negligible. At scale, a `priority_queue` would give O((V+E) log V).

**Q: Dijkstra vs Prim's — what's the difference?**
Both grow from a source by adding nodes. **Dijkstra** minimizes distance from source to each node individually (shortest path tree, can stop early). **Prim's** minimizes total edge weight to connect all nodes (minimum spanning tree, must visit all). Dijkstra uses accumulated distance; Prim's uses edge weight alone.

**Q: How does Prim's avoid cycles?**
The `inMst` set tracks nodes already in the tree. When scanning outgoing edges, we skip any whose destination is already in `inMst`. Since we never revisit a node, we can never create a cycle.

**Q: What triggers surge pricing?**
When `occupiedSeats * 100 > capacity * 80` — i.e., more than 80% of seats are occupied. Integer math avoids floating-point precision issues.

**Q: Why 80% threshold and 1.3× multiplier?**
Mirrors real-world pricing: Uber's surge starts at ~1.2× during high demand. Indian Railways' Tatkal charges a premium for last-minute bookings. 80% is a natural threshold where demand exceeds supply.

**Q: What if source and destination are the same in Dijkstra?**
```cpp
if (src == dst) return result;  // empty path, 0 distance
```
Early exit. No algorithm needed.

**Q: What if there's no path between two stations?**
```cpp
if (!dist.count(dst)) return PathResult{};
```
Returns empty result. API translates to 404 "No path found."

**Q: Why `edgeWeight` returns a non-integer for "shortest" mode?**
```cpp
return static_cast<double>(route.distanceKm);
```
Consistent return type — both modes return double. The cast is a formality since distanceKm is int but the weighted formula returns double.

**Q: Can you trace Dijkstra on your seed data from S1 to S4?**
```
dist[S1]=0
From S1: relax R1(S2,10), R3(S3,40), R6(S5,30)
  dist[S2]=10, dist[S3]=40, dist[S5]=30
Pick S2 (min=10), relax R2(S3,15), R5(S4,25)
  dist[S3]=min(40, 10+15)=25, dist[S4]=10+25=35
Pick S3 (min=25), relax R4(S4,20)
  dist[S4]=min(35, 25+20)=35  (unchanged)
Pick S4 (min=35) → reached dest!
Path: S1→R1→S2→R5→S4, distance=35
```

**Q: What is the "least crowded" path from S1 to S3?**
If R3 (direct, 40km) is 100% full: weight = 40 × (1+1.0) = 80. R1(10km) + R2(15km) if both empty = 10 + 15 = 25. Algorithm picks the two-hop route even though it's longer in distance (25km vs 40km).
