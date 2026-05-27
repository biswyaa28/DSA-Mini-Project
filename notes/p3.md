# Viva Prep — P3: Dijkstra + Prim's MST + Dynamic Pricing

## Your Role
You implemented the three core algorithms: Dijkstra's shortest path (two modes), Prim's Minimum Spanning Tree, and greedy surge pricing. This is the algorithmic heart of the project.

## Key Files You Own

| File | Algorithm | Lines |
|------|-----------|-------|
| `src/routing_engine.cpp` | Dijkstra (2 modes: shortest, least-crowded) | 89 |
| `src/mst_engine.cpp` | Prim's Algorithm | 47 |
| `src/pricing_engine.cpp` | Greedy surge pricing | 13 |

---

## 1. Dijkstra's Algorithm (`routing_engine.cpp`)

### Standard Dijkstra Pseudocode
```
dist[source] = 0
for each node: dist[node] = INF
visited = empty

while (unvisited node with smallest dist exists):
    current = that node
    visited.add(current)
    if current == destination: break
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
  std::unordered_map<std::string, double> dist;       // distances
  std::unordered_map<std::string, std::string> prev;   // previous station
  std::unordered_map<std::string, std::string> routeTaken; // which route used
  std::unordered_set<std::string> visited;

  dist[src] = 0.0;

  while (true) {
    // Find unvisited node with minimum distance (O(V) linear scan)
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
      double w = edgeWeight(route, mode);  // ← KEY: edge weight depends on mode
      double newDist = dist[current] + w;
      if (!dist.count(route.toStationId) || newDist < dist[route.toStationId]) {
        dist[route.toStationId] = newDist;
        prev[route.toStationId] = current;
        routeTaken[route.toStationId] = route.id;
      }
    }
  }

  // Reconstruct path from destination back to source
  if (!dist.count(dst)) return PathResult{};  // no path exists

  std::vector<std::string> path;
  std::string cur = dst;
  while (cur != src) {
    path.push_back(routeTaken[cur]);
    cur = prev[cur];
  }
  result.routeIds.assign(path.rbegin(), path.rend());  // reverse to get source→dest
  result.totalDistance = dist[dst];
  return result;
}
```

### Edge Weight Function (The Key)
```cpp
double RoutingEngine::edgeWeight(const Route& route,
                                  const std::string& mode) const {
  if (mode == "shortest") {
    return static_cast<double>(route.distanceKm);
  }
  // least_crowded: penalize crowded routes
  double occupancyRatio = route.occupiedSeats / route.capacity;
  return route.distanceKm * (1.0 + occupancyRatio);
}
```

| Mode | Edge Weight Formula | Behavior |
|------|-------------------|----------|
| `shortest` | `distanceKm` | Pure shortest path by distance |
| `least_crowded` | `distance × (1 + occupancyRatio)` | Doubles effective distance of full routes |

**Example**: Route A: 10km, 100% full → weight = 10 × (1 + 1.0) = 20
Route B: 15km, 20% full → weight = 15 × (1 + 0.2) = 18
→ Algorithm picks Route B even though it's longer by distance!

---

## 2. Prim's Algorithm (`mst_engine.cpp`)

### Standard Prim's Pseudocode
```
visited = {startNode}
while (there are edges connecting visited to unvisited):
    pick the cheapest such edge
    add the new node to visited
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

    // Scan ALL edges from ALL visited nodes → find cheapest to unvisited
    for (const std::string& node : inMst) {
      for (const Route& route : graph_.outgoing(node)) {
        if (inMst.count(route.toStationId)) continue;  // skip already in MST
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

### Prim's vs Kruskal's — Know This!

| | Prim's (ours) | Kruskal's |
|---|---|---|
| Approach | Grow from one node, add cheapest connecting edge | Sort ALL edges, add cheapest that doesn't create cycle |
| Data Structure | Visited set + scan | Union-Find (Disjoint Set) |
| Best for | Dense graphs | Sparse graphs |
| Start node | Required | Any |

**Why Prim's here?** The railway "minimum cost network" question naturally starts from a specific station. Prim's grows outward from that station — intuitive for a railway context.

---

## 3. Greedy Surge Pricing (`pricing_engine.cpp`)

### The Implementation
```cpp
constexpr double kSurgeMultiplier = 1.3;  // 30% surge

double PricingEngine::currentFare(const Route& route) const {
  if (route.isSurgePricing()) {  // occupied > 80% of capacity
    return route.baseFare * kSurgeMultiplier;
  }
  return route.baseFare;
}
```

Where `isSurgePricing()` (in `models.hpp`):
```cpp
bool Route::isSurgePricing() const {
  if (capacity <= 0) return false;
  return occupiedSeats * 100 > capacity * 80;  // integer math, no floats
}
```

### Why is this Greedy?
A **greedy algorithm** makes the locally optimal choice at each step. Here: "if the train is busy RIGHT NOW, raise the price." It doesn't consider:
- Whether occupancy will go up or down tomorrow
- Historical pricing patterns
- Competitor pricing

It just applies a 1.3× multiplier when occupancy > 80%. Simple and immediate.

### Why Greedy Works Here
Pricing is a threshold decision, not an optimization over time. If the train is >80% full, raise the fare by 30% — that's it. No need for DP or ML.

---

## 4. Complexity Analysis

| Algorithm | Your Complexity | Optimal Complexity |
|-----------|----------------|-------------------|
| Dijkstra | O(V²) — linear min-find | O((V+E) log V) with priority queue |
| Prim's | O(V² × E) — double nested loop | O(E log V) with priority queue |
| Surge Pricing | O(1) | O(1) — can't beat it |

**Why not optimize?** 5 stations, 7 routes. The difference between O(V²) and O((V+E) log V) is ~25 vs ~30 operations. Not worth the code complexity.

---

## Common Viva Questions

**Q: Explain how your Dijkstra handles two different modes?**
The `edgeWeight()` function returns different weights based on the mode string. `shortest` uses raw distance. `least_crowded` uses `distance × (1 + occupancyRatio)` which penalizes crowded routes. The rest of the algorithm is identical.

**Q: What's the time complexity of your Dijkstra?**
O(V²) because we find the minimum unvisited node using a linear scan. With V=5 stations, it's negligible. With a priority queue (`std::priority_queue`), it would be O((V+E) log V).

**Q: What's the difference between Prim's and Dijkstra's?**
Both start from a source and grow outward. But:
- **Dijkstra's** measures distance FROM source to each node individually (shortest path tree)
- **Prim's** measures the MINIMUM total edge weight to connect all nodes (minimum spanning tree)
- Dijkstra can stop early (once destination reached). Prim's must visit all reachable nodes.

**Q: How does your Prim's avoid cycles?**
The `inMst` set tracks all nodes already in the MST. When scanning edges, we skip any edge whose destination is already in `inMst`. This ensures we never create a cycle.

**Q: What's the surge threshold and why 80%?**
80% occupancy triggers a 30% price surge. This mirrors real-world systems like Uber's surge pricing or Indian Railways' Tatkal scheme — higher demand = higher price to manage demand.

**Q: Is there an MST for our seed data?**
With 5 stations, Prim's finds the 4 cheapest routes that connect all stations. The total minimum distance is computed by the algorithm.

**Q: What if there's no path between two stations in Dijkstra?**
```cpp
if (!dist.count(dst)) return PathResult{};
```
Returns an empty result with 0 distance. The API then returns a 404 "No path found."

**Q: Why `PathResult` struct?**
```cpp
struct PathResult {
  std::vector<std::string> routeIds;  // list of route IDs in order
  double totalDistance;                // sum of distances
};
```
Clean return type instead of output parameters or throwing exceptions.
