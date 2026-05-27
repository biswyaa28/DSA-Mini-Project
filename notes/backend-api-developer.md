# Viva Preparation Notes — Backend / API Developer

## Your Role
You built the REST API server that sits between the browser and the C++ engine. You handle all HTTP requests, route them to the right logic modules, and return JSON responses.

## Key Files You Own

| File | Lines | Purpose |
|------|-------|---------|
| `src/main.cpp` | ~10 | Entry point — creates `ApiServer`, calls `configure(8080)` and `run()` |
| `src/api_server.cpp` | 363 | All 13 REST API endpoints + seed data |
| `src/storage_manager.cpp` | 52 | Save/load bookings to JSON files |
| `include/api_server.hpp` | 36 | ApiServer class declaration |

## API Endpoints (13 Total)

### Stations
1. **POST /api/stations** — Add a station `{id, name}`. Returns 409 if duplicate.
2. **GET /api/stations** — List all stations.

### Routes
3. **POST /api/routes** — Add a route `{id, fromStationId, toStationId, distanceKm, baseFare, capacity}`. Returns 404 if stations don't exist, 409 if duplicate route.
4. **GET /api/routes** — List all routes with occupancy info and surge status.

### Bookings
5. **POST /api/bookings** — Book a seat `{routeId, passengerName, isVip, travelDate}`. Calculates fare via `PricingEngine`, books via `BookingEngine`.
6. **POST /api/cancellations** — Cancel `{bookingId}`. Auto-promotes waitlisted passenger.
7. **GET /api/bookings** — List all bookings.
8. **GET /api/waitlist** — List only waitlisted bookings.

### Routing & MST
9. **GET /api/routes/search?from=X&to=Y&mode=shortest|least_crowded** — Dijkstra-based pathfinding.
10. **GET /api/mst?start=X** — Prim's MST report from that station.

### Reports & Custom Features
11. **GET /api/reports/profitability** — Revenue per route.
12. **GET /api/occupancy-map** — Bar-chart ready occupancy % for all routes.
13. **GET /api/fare-trend?routeId=X&days=N** — 7-day fare projection with simulated occupancy growth.

## Common Viva Questions

**Q: How does your API handle errors?**
All endpoints are wrapped in try-catch. Invalid JSON bodies return 400. Not-found returns 404. Duplicates return 409.

**Q: How does the server serve the web UI?**
`svr_.set_mount_point("/", "./web")` — the `web/` folder is served statically. Any path not matching an API route loads the HTML/JS/CSS from there.

**Q: How did you handle CORS?**
Not needed since the browser and API are on the same origin (localhost:8080). The web UI is served as static files by the same server.

**Q: What libraries did you use?**
- `cpp-httplib` (header-only) for the HTTP server
- `nlohmann/json` (header-only) for JSON serialization/deserialization

**Q: How does seed data work?**
`ApiServer` constructor calls `seedSampleData()` which hardcodes 5 stations (Central, West, North, East, South) and 7 routes connecting them. No external file needed to start.

**Q: How would you make this production-ready?**
- Add authentication (JWT tokens)
- Rate limiting
- Database persistence (SQLite instead of JSON file)
- HTTPS support
- Proper logging (instead of cout)

## Architecture Diagram (Mental)
```
Browser (web/) ──HTTP──▶ ApiServer ──▶ StationDirectory
                                 ├──▶ RailGraph
                                 ├──▶ PricingEngine
                                 ├──▶ BookingEngine (+ waitlist queues, refund stack)
                                 ├──▶ RoutingEngine (Dijkstra)
                                 ├──▶ MSTEngine (Prim)
                                 ├──▶ SortingAndSearch
                                 ├──▶ ReportEngine
                                 └──▶ StorageManager (JSON file I/O)
```
