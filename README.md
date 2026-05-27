# Railway Network Management System (DSA Mini Project)

C++ project with REST API + Web UI for railway network management using graphs, Dijkstra, Prim MST, queues, stacks, sorting, and binary search.

## Quick Start

```bash
./run.sh
```

Open http://localhost:8080 in your browser.

## Features

- **Stations & Routes** - Add stations and railway routes with distance/fare/capacity
- **Booking** - Book seats with VIP-first waitlist queue; cancel with refund stack and auto-promotion
- **Routing** - Shortest path (distance) and least-crowded path (modified Dijkstra)
- **MST Report** - Minimum-cost network using Prim's algorithm
- **Dynamic Pricing** - Greedy surge when occupancy > 80%
- **Custom Features**:
  - Seat Occupancy Map per route
  - 7-Day Fare Trend Predictor
- **Sorting & Search** - Multi-level sort by route > date > fare; binary search on fare ranges

## Testing

```bash
ctest --test-dir build -V
```

## Dependencies

- C++17
- CMake 3.16+
- Dependencies are header-only (bundled): cpp-httplib, nlohmann/json

## DSA Concepts Used

| Concept | Implementation |
|---------|---------------|
| Graph (Adjacency List) | RailGraph - station nodes, route edges |
| Dijkstra | RoutingEngine - shortest and least-crowded modes |
| Prim's MST | MSTEngine - minimum-cost network report |
| Queue | VIP-first waitlist (std::deque) |
| Stack | Refund history (std::stack) |
| Sorting | Multi-level sort (route, date, fare) |
| Binary Search | Fare range search on sorted fares |
| Greedy Algorithm | Surge pricing at >80% occupancy |
