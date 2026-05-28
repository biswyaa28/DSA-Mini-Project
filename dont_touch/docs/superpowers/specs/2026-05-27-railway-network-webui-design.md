# Railway Network Management System with Graph Optimization (C++ + Web UI) - Design

Date: 2026-05-27

## 1) Project Goal
Build a demo-friendly local mini project for DSA (C++) where:
- Core logic is implemented in C++ with clear, easy-to-explain classes.
- A REST API connects C++ backend to a browser Web UI.
- One executable runs locally and serves both API and frontend.
- All required features from PS_02 are covered, including at least two custom features.

## 2) Scope and Constraints

### In Scope
- Single C++ app on `localhost:8080`.
- REST API using `cpp-httplib`.
- JSON persistence using `nlohmann/json`.
- Plain HTML/CSS/JS frontend served by backend.
- All listed DSA requirements (#1 to #9).

### Out of Scope
- User authentication/accounts.
- Cloud deployment.
- External database setup.
- Complex ML prediction models.

## 3) Architecture

### High-Level
One process contains:
1. HTTP server (`cpp-httplib`) for API + static files.
2. DSA core modules (graph, booking, pricing, reports).
3. Storage manager (JSON read/write in `data/`).

### Why this architecture
- Simplest to demo: run one command, open browser.
- Clean viva explanation: UI -> API -> DSA modules -> JSON persistence.
- No additional service orchestration.

## 4) Core Modules (C++)

### 4.1 StationDirectory
- Maintains station records using array/list-backed structures.
- Provides quick lookup and validation for station IDs/names.
- Supports sorted views for binary-search-related operations.

### 4.2 RailGraph
- Adjacency list representation.
- Node: station.
- Edge: route with `distance`, `baseFare`, `capacity`, `occupiedSeats`.
- Add/update/remove route operations.

### 4.3 BookingEngine
- Handles seat booking and cancellation.
- VIP-first waitlist queue behavior.
- Refund history stack.
- Auto-promotion from waitlist on cancellation.

### 4.4 RoutingEngine
- Modified Dijkstra with two modes:
  - `shortest`: distance-focused path.
  - `least_crowded`: occupancy penalty in edge weight.

### 4.5 MSTEngine
- Prim's algorithm for network optimization report.
- Produces MST edges + total optimization cost metric.

### 4.6 PricingEngine
- Greedy surge logic.
- If route occupancy > 80%, applies fare multiplier.

### 4.7 SortingAndSearch
- Multi-level sorting by route -> date -> fare.
- Binary search on sorted fare values/ranges.

### 4.8 ReportEngine
- Route profitability summaries.
- Occupancy and cancellation metrics.

### 4.9 Custom Features
1. Seat occupancy map per route (visualizable in UI).
2. 7-day fare trend predictor using simple moving-average style calculation.

## 5) Requirement-to-Implementation Mapping

1. Add train/route (distance, fare, capacity)
   - `RailGraph::addRoute`, `RailGraph::updateRoute`
2. Book seat + waitlist (VIP before regular)
   - `BookingEngine` with prioritized queue logic
3. Dijkstra shortest + least crowded
   - `RoutingEngine::findPath(mode)`
4. MST using Prim's
   - `MSTEngine::buildMST()`
5. Cascading cancellation (stack + queue)
   - cancellation pushes refund record to stack, triggers queue promotion
6. Dynamic pricing surge > 80%
   - `PricingEngine::currentFare(route)`
7. Multi-level sorting route -> date -> fare
   - `SortingAndSearch::sortTickets(...)`
8. Binary search on fare ranges + station lookup structure
   - `SortingAndSearch::binarySearchFareRange(...)`, `StationDirectory`
9. Two custom features
   - occupancy map + fare trend predictor

## 6) REST API Contract

All responses use shape:
`{ "success": true|false, "message": "...", "data": ... }`

### Core endpoints
- `POST /api/stations`
- `GET /api/stations`
- `POST /api/routes`
- `GET /api/routes`
- `POST /api/bookings`
- `POST /api/cancellations`
- `GET /api/waitlist`
- `GET /api/routes/search?from=A&to=B&mode=shortest|least_crowded`
- `GET /api/mst`
- `GET /api/reports/profitability`
- `GET /api/fares/range?min=100&max=500`
- `GET /api/occupancy-map`
- `GET /api/fare-trend?routeId=R1&days=7`

### Status code policy
- `200`: success
- `400`: invalid input
- `404`: resource not found
- `409`: conflict/full/duplicate
- `500`: unexpected server error

## 7) Web UI Design

Single-page, tab-based UI with sections:
- Stations/Routes management
- Booking/Waitlist
- Route search (two Dijkstra modes)
- MST and reports
- Dynamic pricing + fare range filter
- Seat occupancy map
- Fare trend viewer

Frontend uses `fetch` to call API and renders tables/cards/charts without page reload.

## 8) Data Model (Persistent JSON)

Files in `data/`:
- `stations.json`
- `routes.json`
- `bookings.json`
- `waitlist.json`
- `refunds.json`
- `analytics.json`

Lifecycle:
- On startup: load all files if present.
- On write operations: save updated files.
- On graceful shutdown: final save checkpoint.

## 9) Error Handling Strategy

- Validate all incoming payload fields.
- Guard conditions in service layer for missing stations/routes.
- Return user-friendly messages for demo clarity.
- Keep internal exceptions mapped to safe API responses.

## 10) Testing and Demo Verification

### Unit/logic checks
- Graph creation and route insertion
- Dijkstra both modes
- Prim MST output sanity
- Waitlist VIP ordering
- Refund stack push/pop behavior
- Surge pricing trigger above 80%
- Multi-level sort and fare-range binary search

### Integration checks
- API route flow: add -> book -> cancel -> promote
- API/UI sync correctness
- Persistence reload correctness

### Demo script (5-7 minutes)
1. Add stations and routes
2. Fill seats and show VIP waitlist
3. Cancel booking and show auto-promotion + refund history
4. Compare shortest vs least-crowded path
5. Show MST and profitability report
6. Show occupancy map and 7-day fare trend

## 11) Project Structure (Planned)

```
src/
  main.cpp
  api/ApiServer.cpp
  core/StationDirectory.cpp
  core/RailGraph.cpp
  core/BookingEngine.cpp
  core/RoutingEngine.cpp
  core/MSTEngine.cpp
  core/PricingEngine.cpp
  core/SortingAndSearch.cpp
  core/ReportEngine.cpp
  storage/StorageManager.cpp
include/
  ...headers...
web/
  index.html
  styles.css
  app.js
data/
  *.json
```

## 12) Rationale for Simplicity

- Uses familiar DSA structures directly tied to rubric.
- Avoids overengineering and external infrastructure.
- Keeps explanations straightforward for viva and report writing.
