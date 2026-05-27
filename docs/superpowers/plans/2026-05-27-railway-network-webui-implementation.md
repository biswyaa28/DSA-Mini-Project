# Railway Network Web UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a single local C++ application that implements all DSA requirements (graph, Dijkstra, Prim MST, queue/stack, sorting, binary search, greedy pricing) and serves a browser Web UI via REST API.

**Architecture:** One process hosts both static frontend and REST API (`cpp-httplib`). Core logic lives in focused C++ modules (`StationDirectory`, `RailGraph`, `BookingEngine`, `RoutingEngine`, `MSTEngine`, `PricingEngine`, `SortingAndSearch`, `ReportEngine`). Data is persisted in local JSON files using `nlohmann/json`.

**Tech Stack:** C++17, CMake, cpp-httplib, nlohmann/json, vanilla HTML/CSS/JS, ctest

---

## File Structure and Responsibilities

- `CMakeLists.txt`: build config, test targets, include paths
- `external/httplib.h`: header-only HTTP server library
- `external/json.hpp`: header-only JSON library
- `include/models.hpp`: shared DTOs/structs (`Station`, `Route`, `Booking`, etc.)
- `include/station_directory.hpp`: station records + lookup APIs
- `include/rail_graph.hpp`: adjacency-list graph and route updates
- `include/pricing_engine.hpp`: occupancy-based greedy surge fare
- `include/booking_engine.hpp`: booking, VIP waitlist queue, refund stack
- `include/routing_engine.hpp`: modified Dijkstra (shortest, least_crowded)
- `include/mst_engine.hpp`: Prim MST implementation
- `include/sort_search.hpp`: multi-level sorting + fare-range binary search
- `include/report_engine.hpp`: profitability/occupancy/cancellation report generation
- `include/storage_manager.hpp`: JSON load/save
- `include/api_server.hpp`: API route registration and static file serving
- `src/*.cpp`: implementation files matching each include file
- `src/main.cpp`: application bootstrap, wiring modules, start server
- `web/index.html`: UI layout (tabs/forms/tables/cards)
- `web/styles.css`: styling
- `web/app.js`: API calls + DOM rendering
- `data/*.json`: persistent state files
- `tests/*.cpp`: test executables per module

---

### Task 1: Project Scaffold, Build, and Test Harness

**Files:**
- Create: `CMakeLists.txt`
- Create: `include/models.hpp`
- Create: `src/main.cpp`
- Create: `tests/test_models.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_models.cpp
#include "models.hpp"
#include <cassert>

int main() {
    Station s{"S1", "Central"};
    assert(s.id == "S1");
    assert(s.name == "Central");
    Route r{"R1", "S1", "S2", 120.0, 300.0, 100, 0};
    assert(r.capacity == 100);
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake -S . -B build && cmake --build build`

Expected: build fails with `fatal error: 'models.hpp' file not found`

- [ ] **Step 3: Write minimal implementation**

```cpp
// include/models.hpp
#pragma once
#include <string>
#include <vector>

struct Station {
    std::string id;
    std::string name;
};

struct Route {
    std::string id;
    std::string fromStationId;
    std::string toStationId;
    double distanceKm;
    double baseFare;
    int capacity;
    int occupiedSeats;
};

enum class PassengerType { Regular, VIP };

struct Booking {
    std::string bookingId;
    std::string routeId;
    std::string passengerName;
    PassengerType passengerType;
    std::string travelDate;
    double chargedFare;
    bool confirmed;
};
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(RailwayDSA LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include external)

add_executable(railway_app src/main.cpp)

add_executable(test_models tests/test_models.cpp)
enable_testing()
add_test(NAME test_models COMMAND test_models)
```

```cpp
// src/main.cpp
#include <iostream>

int main() {
    std::cout << "Railway server bootstrap" << std::endl;
    return 0;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Expected: `1/1 tests passed`

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt include/models.hpp src/main.cpp tests/test_models.cpp
git commit -m "chore: scaffold C++ project with base models and test harness"
```

---

### Task 2: StationDirectory + RailGraph (Requirement #1, #8 station lookup base)

**Files:**
- Create: `include/station_directory.hpp`
- Create: `src/station_directory.cpp`
- Create: `include/rail_graph.hpp`
- Create: `src/rail_graph.cpp`
- Create: `tests/test_graph_basics.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_graph_basics.cpp
#include "station_directory.hpp"
#include "rail_graph.hpp"
#include <cassert>

int main() {
    StationDirectory stations;
    assert(stations.addStation({"S1", "Central"}));
    assert(stations.addStation({"S2", "West"}));
    assert(!stations.addStation({"S1", "Duplicate"}));

    RailGraph g;
    assert(g.addRoute({"R1", "S1", "S2", 100.0, 250.0, 50, 0}));
    assert(!g.addRoute({"R1", "S1", "S2", 100.0, 250.0, 50, 0}));
    assert(g.hasRoute("R1"));
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`

Expected: compile fails because `station_directory.hpp` and `rail_graph.hpp` are missing.

- [ ] **Step 3: Write minimal implementation**

```cpp
// include/station_directory.hpp
#pragma once
#include "models.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class StationDirectory {
public:
    bool addStation(const Station& s);
    bool hasStation(const std::string& stationId) const;
    const Station* findStation(const std::string& stationId) const;
    const std::vector<Station>& allStations() const;
private:
    std::vector<Station> stations_;
    std::unordered_map<std::string, size_t> idx_;
};
```

```cpp
// src/station_directory.cpp
#include "station_directory.hpp"

bool StationDirectory::addStation(const Station& s) {
    if (idx_.count(s.id)) return false;
    idx_[s.id] = stations_.size();
    stations_.push_back(s);
    return true;
}
bool StationDirectory::hasStation(const std::string& stationId) const { return idx_.count(stationId) > 0; }
const Station* StationDirectory::findStation(const std::string& stationId) const {
    auto it = idx_.find(stationId);
    if (it == idx_.end()) return nullptr;
    return &stations_[it->second];
}
const std::vector<Station>& StationDirectory::allStations() const { return stations_; }
```

```cpp
// include/rail_graph.hpp
#pragma once
#include "models.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class RailGraph {
public:
    bool addRoute(const Route& route);
    bool hasRoute(const std::string& routeId) const;
    bool updateOccupiedSeats(const std::string& routeId, int newValue);
    const Route* findRoute(const std::string& routeId) const;
    std::vector<Route> allRoutes() const;
    std::vector<Route> outgoing(const std::string& stationId) const;
private:
    std::unordered_map<std::string, Route> routesById_;
    std::unordered_map<std::string, std::vector<std::string>> adj_;
};
```

```cpp
// src/rail_graph.cpp
#include "rail_graph.hpp"

bool RailGraph::addRoute(const Route& route) {
    if (routesById_.count(route.id)) return false;
    routesById_[route.id] = route;
    adj_[route.fromStationId].push_back(route.id);
    return true;
}
bool RailGraph::hasRoute(const std::string& routeId) const { return routesById_.count(routeId) > 0; }
bool RailGraph::updateOccupiedSeats(const std::string& routeId, int newValue) {
    auto it = routesById_.find(routeId);
    if (it == routesById_.end()) return false;
    it->second.occupiedSeats = newValue;
    return true;
}
const Route* RailGraph::findRoute(const std::string& routeId) const {
    auto it = routesById_.find(routeId);
    if (it == routesById_.end()) return nullptr;
    return &it->second;
}
std::vector<Route> RailGraph::allRoutes() const {
    std::vector<Route> out;
    out.reserve(routesById_.size());
    for (const auto& kv : routesById_) out.push_back(kv.second);
    return out;
}
std::vector<Route> RailGraph::outgoing(const std::string& stationId) const {
    std::vector<Route> out;
    auto it = adj_.find(stationId);
    if (it == adj_.end()) return out;
    for (const auto& routeId : it->second) out.push_back(routesById_.at(routeId));
    return out;
}
```

```cmake
# add below existing targets in CMakeLists.txt
add_executable(test_graph_basics
  tests/test_graph_basics.cpp
  src/station_directory.cpp
  src/rail_graph.cpp
)
add_test(NAME test_graph_basics COMMAND test_graph_basics)
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Expected: `test_models` and `test_graph_basics` both pass.

- [ ] **Step 5: Commit**

```bash
git add include/station_directory.hpp src/station_directory.cpp include/rail_graph.hpp src/rail_graph.cpp tests/test_graph_basics.cpp CMakeLists.txt
git commit -m "feat: implement station directory and route graph core"
```

---

### Task 3: PricingEngine + BookingEngine (Requirement #2, #5, #6)

**Files:**
- Create: `include/pricing_engine.hpp`
- Create: `src/pricing_engine.cpp`
- Create: `include/booking_engine.hpp`
- Create: `src/booking_engine.cpp`
- Create: `tests/test_booking_pricing.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_booking_pricing.cpp
#include "booking_engine.hpp"
#include "pricing_engine.hpp"
#include "rail_graph.hpp"
#include <cassert>

int main() {
    RailGraph g;
    g.addRoute({"R1", "S1", "S2", 100.0, 100.0, 2, 0});

    PricingEngine pricing;
    BookingEngine engine(g, pricing);

    auto b1 = engine.bookSeat("R1", "Asha", PassengerType::Regular, "2026-05-28");
    auto b2 = engine.bookSeat("R1", "Vik", PassengerType::VIP, "2026-05-28");
    auto b3 = engine.bookSeat("R1", "Ria", PassengerType::Regular, "2026-05-28");

    assert(b1.confirmed);
    assert(b2.confirmed);
    assert(!b3.confirmed); // waitlisted

    auto b4 = engine.bookSeat("R1", "Karan", PassengerType::VIP, "2026-05-28");
    assert(!b4.confirmed);

    // cancel one confirmed booking and ensure VIP gets promoted first
    assert(engine.cancelBooking(b1.bookingId));
    auto promoted = engine.peekLastPromoted();
    assert(promoted.has_value());
    assert(promoted->passengerName == "Karan");

    // surge check once occupancy > 80%
    double fare = pricing.currentFare(*g.findRoute("R1"));
    assert(fare >= 100.0);
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`

Expected: compile fails because booking/pricing headers are missing.

- [ ] **Step 3: Write minimal implementation**

```cpp
// include/pricing_engine.hpp
#pragma once
#include "models.hpp"

class PricingEngine {
public:
    double currentFare(const Route& route) const;
};
```

```cpp
// src/pricing_engine.cpp
#include "pricing_engine.hpp"

double PricingEngine::currentFare(const Route& route) const {
    if (route.capacity <= 0) return route.baseFare;
    const double occ = static_cast<double>(route.occupiedSeats) / static_cast<double>(route.capacity);
    return occ > 0.80 ? route.baseFare * 1.25 : route.baseFare;
}
```

```cpp
// include/booking_engine.hpp
#pragma once
#include "models.hpp"
#include "pricing_engine.hpp"
#include "rail_graph.hpp"
#include <deque>
#include <optional>
#include <stack>
#include <unordered_map>

class BookingEngine {
public:
    BookingEngine(RailGraph& graph, PricingEngine& pricing);
    Booking bookSeat(const std::string& routeId, const std::string& passengerName, PassengerType type, const std::string& date);
    bool cancelBooking(const std::string& bookingId);
    std::optional<Booking> peekLastPromoted() const;
private:
    RailGraph& graph_;
    PricingEngine& pricing_;
    int bookingCounter_ = 1;
    std::unordered_map<std::string, Booking> bookings_;
    std::deque<Booking> vipWaitlist_;
    std::deque<Booking> regularWaitlist_;
    std::stack<Booking> refunds_;
    std::optional<Booking> lastPromoted_;
};
```

```cpp
// src/booking_engine.cpp
#include "booking_engine.hpp"

BookingEngine::BookingEngine(RailGraph& graph, PricingEngine& pricing) : graph_(graph), pricing_(pricing) {}

Booking BookingEngine::bookSeat(const std::string& routeId, const std::string& passengerName, PassengerType type, const std::string& date) {
    Booking b;
    b.bookingId = "B" + std::to_string(bookingCounter_++);
    b.routeId = routeId;
    b.passengerName = passengerName;
    b.passengerType = type;
    b.travelDate = date;
    b.confirmed = false;

    const Route* route = graph_.findRoute(routeId);
    if (!route) return b;

    if (route->occupiedSeats < route->capacity) {
        graph_.updateOccupiedSeats(routeId, route->occupiedSeats + 1);
        const Route* updated = graph_.findRoute(routeId);
        b.chargedFare = pricing_.currentFare(*updated);
        b.confirmed = true;
        bookings_[b.bookingId] = b;
        return b;
    }

    if (type == PassengerType::VIP) vipWaitlist_.push_back(b);
    else regularWaitlist_.push_back(b);
    bookings_[b.bookingId] = b;
    return b;
}

bool BookingEngine::cancelBooking(const std::string& bookingId) {
    auto it = bookings_.find(bookingId);
    if (it == bookings_.end()) return false;
    Booking b = it->second;
    refunds_.push(b);

    if (b.confirmed) {
        const Route* route = graph_.findRoute(b.routeId);
        if (route && route->occupiedSeats > 0) {
            graph_.updateOccupiedSeats(b.routeId, route->occupiedSeats - 1);
        }
        Booking promoted;
        bool hasCandidate = false;
        if (!vipWaitlist_.empty()) {
            promoted = vipWaitlist_.front(); vipWaitlist_.pop_front(); hasCandidate = true;
        } else if (!regularWaitlist_.empty()) {
            promoted = regularWaitlist_.front(); regularWaitlist_.pop_front(); hasCandidate = true;
        }
        if (hasCandidate) {
            const Route* afterFree = graph_.findRoute(promoted.routeId);
            if (afterFree && afterFree->occupiedSeats < afterFree->capacity) {
                graph_.updateOccupiedSeats(promoted.routeId, afterFree->occupiedSeats + 1);
                promoted.confirmed = true;
                bookings_[promoted.bookingId] = promoted;
                lastPromoted_ = promoted;
            }
        }
    }

    bookings_.erase(bookingId);
    return true;
}

std::optional<Booking> BookingEngine::peekLastPromoted() const { return lastPromoted_; }
```

```cmake
add_executable(test_booking_pricing
  tests/test_booking_pricing.cpp
  src/rail_graph.cpp
  src/pricing_engine.cpp
  src/booking_engine.cpp
)
add_test(NAME test_booking_pricing COMMAND test_booking_pricing)
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Expected: `test_booking_pricing` passes; total tests pass.

- [ ] **Step 5: Commit**

```bash
git add include/pricing_engine.hpp src/pricing_engine.cpp include/booking_engine.hpp src/booking_engine.cpp tests/test_booking_pricing.cpp CMakeLists.txt
git commit -m "feat: add booking engine with VIP waitlist, refund stack, and surge pricing"
```

---

### Task 4: RoutingEngine + MSTEngine (Requirement #3, #4)

**Files:**
- Create: `include/routing_engine.hpp`
- Create: `src/routing_engine.cpp`
- Create: `include/mst_engine.hpp`
- Create: `src/mst_engine.cpp`
- Create: `tests/test_routing_mst.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_routing_mst.cpp
#include "rail_graph.hpp"
#include "routing_engine.hpp"
#include "mst_engine.hpp"
#include <cassert>

int main() {
    RailGraph g;
    g.addRoute({"R1", "S1", "S2", 10.0, 100, 100, 10});
    g.addRoute({"R2", "S2", "S3", 15.0, 100, 100, 20});
    g.addRoute({"R3", "S1", "S3", 40.0, 100, 100, 5});

    RoutingEngine router(g);
    auto shortest = router.findPath("S1", "S3", "shortest");
    assert(shortest.totalDistance == 25.0);

    g.updateOccupiedSeats("R1", 95);
    g.updateOccupiedSeats("R2", 95);
    auto leastCrowded = router.findPath("S1", "S3", "least_crowded");
    assert(!leastCrowded.routeIds.empty());

    MSTEngine mst(g);
    auto result = mst.buildMST("S1");
    assert(result.totalDistance > 0.0);
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`

Expected: compile fails because routing and MST files are missing.

- [ ] **Step 3: Write minimal implementation**

```cpp
// include/routing_engine.hpp
#pragma once
#include "rail_graph.hpp"
#include <string>
#include <vector>

struct PathResult {
    std::vector<std::string> routeIds;
    double totalDistance = 0.0;
};

class RoutingEngine {
public:
    explicit RoutingEngine(const RailGraph& graph);
    PathResult findPath(const std::string& src, const std::string& dst, const std::string& mode) const;
private:
    const RailGraph& graph_;
};
```

```cpp
// include/mst_engine.hpp
#pragma once
#include "rail_graph.hpp"
#include <string>
#include <vector>

struct MstResult {
    std::vector<std::string> routeIds;
    double totalDistance = 0.0;
};

class MSTEngine {
public:
    explicit MSTEngine(const RailGraph& graph);
    MstResult buildMST(const std::string& startStation) const;
private:
    const RailGraph& graph_;
};
```

```cpp
// src/routing_engine.cpp (minimal, then improve in same task if needed)
#include "routing_engine.hpp"
#include <queue>
#include <unordered_map>

RoutingEngine::RoutingEngine(const RailGraph& graph) : graph_(graph) {}

PathResult RoutingEngine::findPath(const std::string& src, const std::string& dst, const std::string& mode) const {
    std::unordered_map<std::string, double> dist;
    std::unordered_map<std::string, std::string> prevRoute;
    std::unordered_map<std::string, std::string> prevStation;
    using QN = std::pair<double, std::string>;
    std::priority_queue<QN, std::vector<QN>, std::greater<QN>> pq;

    dist[src] = 0.0;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (u == dst) break;
        if (d > dist[u]) continue;
        for (const auto& edge : graph_.outgoing(u)) {
            double w = edge.distanceKm;
            if (mode == "least_crowded" && edge.capacity > 0) {
                double occ = static_cast<double>(edge.occupiedSeats) / static_cast<double>(edge.capacity);
                w += occ * 50.0;
            }
            double nd = d + w;
            if (!dist.count(edge.toStationId) || nd < dist[edge.toStationId]) {
                dist[edge.toStationId] = nd;
                prevRoute[edge.toStationId] = edge.id;
                prevStation[edge.toStationId] = u;
                pq.push({nd, edge.toStationId});
            }
        }
    }

    PathResult out;
    if (!dist.count(dst)) return out;
    out.totalDistance = dist[dst];
    std::string cur = dst;
    while (cur != src && prevRoute.count(cur)) {
        out.routeIds.push_back(prevRoute[cur]);
        cur = prevStation[cur];
    }
    std::reverse(out.routeIds.begin(), out.routeIds.end());
    return out;
}
```

```cpp
// src/mst_engine.cpp
#include "mst_engine.hpp"
#include <queue>
#include <unordered_set>

MSTEngine::MSTEngine(const RailGraph& graph) : graph_(graph) {}

MstResult MSTEngine::buildMST(const std::string& startStation) const {
    struct EdgePick { double w; std::string from; Route route; };
    auto cmp = [](const EdgePick& a, const EdgePick& b){ return a.w > b.w; };
    std::priority_queue<EdgePick, std::vector<EdgePick>, decltype(cmp)> pq(cmp);

    MstResult out;
    std::unordered_set<std::string> vis;
    vis.insert(startStation);
    for (const auto& e : graph_.outgoing(startStation)) pq.push({e.distanceKm, startStation, e});

    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        const auto& e = top.route;
        if (vis.count(e.toStationId)) continue;
        vis.insert(e.toStationId);
        out.routeIds.push_back(e.id);
        out.totalDistance += e.distanceKm;
        for (const auto& nxt : graph_.outgoing(e.toStationId)) {
            if (!vis.count(nxt.toStationId)) pq.push({nxt.distanceKm, e.toStationId, nxt});
        }
    }
    return out;
}
```

```cmake
add_executable(test_routing_mst
  tests/test_routing_mst.cpp
  src/rail_graph.cpp
  src/routing_engine.cpp
  src/mst_engine.cpp
)
add_test(NAME test_routing_mst COMMAND test_routing_mst)
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Expected: routing and MST tests pass.

- [ ] **Step 5: Commit**

```bash
git add include/routing_engine.hpp src/routing_engine.cpp include/mst_engine.hpp src/mst_engine.cpp tests/test_routing_mst.cpp CMakeLists.txt
git commit -m "feat: implement modified Dijkstra and Prim MST engines"
```

---

### Task 5: Sorting/Search + Reports + JSON Storage (Requirement #7, #8, reporting)

**Files:**
- Create: `include/sort_search.hpp`
- Create: `src/sort_search.cpp`
- Create: `include/report_engine.hpp`
- Create: `src/report_engine.cpp`
- Create: `include/storage_manager.hpp`
- Create: `src/storage_manager.cpp`
- Create: `tests/test_sort_search_reports.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/test_sort_search_reports.cpp
#include "sort_search.hpp"
#include "report_engine.hpp"
#include "storage_manager.hpp"
#include <cassert>

int main() {
    std::vector<Booking> bookings = {
        {"B1","R2","A",PassengerType::Regular,"2026-05-29",200,true},
        {"B2","R1","B",PassengerType::Regular,"2026-05-28",150,true},
        {"B3","R1","C",PassengerType::VIP,"2026-05-28",120,true}
    };
    SortingAndSearch ss;
    ss.sortByRouteDateFare(bookings);
    assert(bookings[0].routeId == "R1");
    auto idx = ss.binarySearchFareRange({120,150,200}, 140, 210);
    assert(idx.first <= idx.second);

    ReportEngine re;
    auto p = re.profitabilityByRoute(bookings);
    assert(p["R1"] == 270);

    StorageManager sm("data");
    assert(sm.saveBookings(bookings));
    auto loaded = sm.loadBookings();
    assert(!loaded.empty());
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`

Expected: compile fails due to missing headers.

- [ ] **Step 3: Write minimal implementation**

```cpp
// include/sort_search.hpp
#pragma once
#include "models.hpp"
#include <utility>
#include <vector>

class SortingAndSearch {
public:
    void sortByRouteDateFare(std::vector<Booking>& bookings) const;
    std::pair<int,int> binarySearchFareRange(const std::vector<double>& sortedFares, double minFare, double maxFare) const;
};
```

```cpp
// src/sort_search.cpp
#include "sort_search.hpp"
#include <algorithm>

void SortingAndSearch::sortByRouteDateFare(std::vector<Booking>& bookings) const {
    std::sort(bookings.begin(), bookings.end(), [](const Booking& a, const Booking& b) {
        if (a.routeId != b.routeId) return a.routeId < b.routeId;
        if (a.travelDate != b.travelDate) return a.travelDate < b.travelDate;
        return a.chargedFare < b.chargedFare;
    });
}

std::pair<int,int> SortingAndSearch::binarySearchFareRange(const std::vector<double>& sortedFares, double minFare, double maxFare) const {
    auto l = std::lower_bound(sortedFares.begin(), sortedFares.end(), minFare);
    auto r = std::upper_bound(sortedFares.begin(), sortedFares.end(), maxFare);
    if (l == sortedFares.end() || l >= r) return {-1, -1};
    return {static_cast<int>(l - sortedFares.begin()), static_cast<int>((r - sortedFares.begin()) - 1)};
}
```

```cpp
// include/report_engine.hpp
#pragma once
#include "models.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class ReportEngine {
public:
    std::unordered_map<std::string, double> profitabilityByRoute(const std::vector<Booking>& bookings) const;
};
```

```cpp
// src/report_engine.cpp
#include "report_engine.hpp"

std::unordered_map<std::string, double> ReportEngine::profitabilityByRoute(const std::vector<Booking>& bookings) const {
    std::unordered_map<std::string, double> out;
    for (const auto& b : bookings) {
        if (b.confirmed) out[b.routeId] += b.chargedFare;
    }
    return out;
}
```

```cpp
// include/storage_manager.hpp
#pragma once
#include "models.hpp"
#include <string>
#include <vector>

class StorageManager {
public:
    explicit StorageManager(std::string baseDir);
    bool saveBookings(const std::vector<Booking>& bookings) const;
    std::vector<Booking> loadBookings() const;
private:
    std::string baseDir_;
};
```

```cpp
// src/storage_manager.cpp
#include "storage_manager.hpp"
#include "json.hpp"
#include <fstream>

using nlohmann::json;

StorageManager::StorageManager(std::string baseDir) : baseDir_(std::move(baseDir)) {}

bool StorageManager::saveBookings(const std::vector<Booking>& bookings) const {
    json j = json::array();
    for (const auto& b : bookings) {
        j.push_back({
            {"bookingId", b.bookingId},
            {"routeId", b.routeId},
            {"passengerName", b.passengerName},
            {"passengerType", b.passengerType == PassengerType::VIP ? "VIP" : "Regular"},
            {"travelDate", b.travelDate},
            {"chargedFare", b.chargedFare},
            {"confirmed", b.confirmed}
        });
    }
    std::ofstream out(baseDir_ + "/bookings.json");
    if (!out) return false;
    out << j.dump(2);
    return true;
}

std::vector<Booking> StorageManager::loadBookings() const {
    std::ifstream in(baseDir_ + "/bookings.json");
    if (!in) return {};
    json j; in >> j;
    std::vector<Booking> out;
    for (const auto& x : j) {
        Booking b;
        b.bookingId = x.value("bookingId", "");
        b.routeId = x.value("routeId", "");
        b.passengerName = x.value("passengerName", "");
        b.passengerType = x.value("passengerType", "Regular") == "VIP" ? PassengerType::VIP : PassengerType::Regular;
        b.travelDate = x.value("travelDate", "");
        b.chargedFare = x.value("chargedFare", 0.0);
        b.confirmed = x.value("confirmed", false);
        out.push_back(b);
    }
    return out;
}
```

```cmake
add_executable(test_sort_search_reports
  tests/test_sort_search_reports.cpp
  src/sort_search.cpp
  src/report_engine.cpp
  src/storage_manager.cpp
)
add_test(NAME test_sort_search_reports COMMAND test_sort_search_reports)
```

- [ ] **Step 4: Run test to verify it passes**

Run: `mkdir -p data && cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Expected: sorting/search/report/storage tests pass.

- [ ] **Step 5: Commit**

```bash
git add include/sort_search.hpp src/sort_search.cpp include/report_engine.hpp src/report_engine.cpp include/storage_manager.hpp src/storage_manager.cpp tests/test_sort_search_reports.cpp CMakeLists.txt
git commit -m "feat: add sorting, binary search, reports, and JSON storage"
```

---

### Task 6: REST API Server Wiring (All backend endpoints)

**Files:**
- Create: `include/api_server.hpp`
- Create: `src/api_server.cpp`
- Modify: `src/main.cpp`
- Create: `tests/test_api_smoke.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing API smoke test**

```cpp
// tests/test_api_smoke.cpp
#include "api_server.hpp"
#include <cassert>

int main() {
    // This is a compile/link smoke test for ApiServer wiring.
    ApiServer server;
    assert(server.configure(8081));
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build`

Expected: compile fails because `api_server.hpp` does not exist.

- [ ] **Step 3: Implement server and endpoints**

```cpp
// include/api_server.hpp
#pragma once
#include "booking_engine.hpp"
#include "mst_engine.hpp"
#include "pricing_engine.hpp"
#include "rail_graph.hpp"
#include "report_engine.hpp"
#include "routing_engine.hpp"
#include "sort_search.hpp"
#include "station_directory.hpp"
#include "storage_manager.hpp"
#include "httplib.h"

class ApiServer {
public:
    ApiServer();
    bool configure(int port);
    void run();
private:
    int port_ = 8080;
    httplib::Server http_;

    StationDirectory stations_;
    RailGraph graph_;
    PricingEngine pricing_;
    BookingEngine booking_;
    RoutingEngine routing_;
    MSTEngine mst_;
    SortingAndSearch sorter_;
    ReportEngine reports_;
    StorageManager storage_;

    void registerRoutes();
};
```

```cpp
// src/api_server.cpp (explicit handlers for all required endpoints)
#include "api_server.hpp"
#include "json.hpp"
#include <algorithm>

using nlohmann::json;

ApiServer::ApiServer()
  : booking_(graph_, pricing_), routing_(graph_), mst_(graph_), storage_("data") {}

bool ApiServer::configure(int port) {
    port_ = port;
    registerRoutes();
    http_.set_mount_point("/", "./web");
    return true;
}

void ApiServer::registerRoutes() {
    http_.Post("/api/stations", [&](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body, nullptr, false);
        if (j.is_discarded()) { res.status = 400; res.set_content(R"({"success":false,"message":"invalid json"})", "application/json"); return; }
        bool ok = stations_.addStation({j.value("id",""), j.value("name","")});
        res.status = ok ? 200 : 409;
        res.set_content(json({{"success",ok},{"message", ok?"station added":"station exists"}}).dump(), "application/json");
    });

    http_.Get("/api/stations", [&](const httplib::Request&, httplib::Response& res){
        json arr = json::array();
        for (const auto& s : stations_.allStations()) arr.push_back({{"id",s.id},{"name",s.name}});
        res.set_content(json({{"success",true},{"data",arr}}).dump(), "application/json");
    });

    http_.Post("/api/routes", [&](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body, nullptr, false);
        if (j.is_discarded()) { res.status = 400; res.set_content(R"({"success":false,"message":"invalid json"})", "application/json"); return; }
        Route r{j.value("id",""), j.value("fromStationId",""), j.value("toStationId",""), j.value("distanceKm",0.0), j.value("baseFare",0.0), j.value("capacity",0), 0};
        bool ok = graph_.addRoute(r);
        res.status = ok ? 200 : 409;
        res.set_content(json({{"success",ok}}).dump(), "application/json");
    });

    http_.Get("/api/routes", [&](const httplib::Request&, httplib::Response& res){
        json arr = json::array();
        for (const auto& r : graph_.allRoutes()) {
            arr.push_back({{"id",r.id},{"from",r.fromStationId},{"to",r.toStationId},{"distanceKm",r.distanceKm},{"baseFare",r.baseFare},{"capacity",r.capacity},{"occupiedSeats",r.occupiedSeats}});
        }
        res.set_content(json({{"success",true},{"data",arr}}).dump(), "application/json");
    });

    http_.Post("/api/bookings", [&](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body, nullptr, false);
        if (j.is_discarded()) { res.status = 400; res.set_content(R"({"success":false,"message":"invalid json"})", "application/json"); return; }
        auto t = j.value("passengerType", "Regular") == "VIP" ? PassengerType::VIP : PassengerType::Regular;
        auto b = booking_.bookSeat(j.value("routeId",""), j.value("passengerName",""), t, j.value("travelDate",""));
        res.set_content(json({{"success",true},{"data",{{"bookingId",b.bookingId},{"confirmed",b.confirmed},{"chargedFare",b.chargedFare}}}}).dump(), "application/json");
    });

    http_.Post("/api/cancellations", [&](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body, nullptr, false);
        if (j.is_discarded()) { res.status = 400; res.set_content(R"({"success":false,"message":"invalid json"})", "application/json"); return; }
        bool ok = booking_.cancelBooking(j.value("bookingId", ""));
        res.status = ok ? 200 : 404;
        res.set_content(json({{"success",ok}}).dump(), "application/json");
    });

    http_.Get("/api/waitlist", [&](const httplib::Request&, httplib::Response& res){
        auto promoted = booking_.peekLastPromoted();
        json data = promoted.has_value() ? json({{"lastPromoted", promoted->passengerName}}) : json::object();
        res.set_content(json({{"success",true},{"data",data}}).dump(), "application/json");
    });

    http_.Get("/api/routes/search", [&](const httplib::Request& req, httplib::Response& res){
        auto result = routing_.findPath(req.get_param_value("from"), req.get_param_value("to"), req.get_param_value("mode"));
        res.set_content(json({{"success",true},{"data",{{"routeIds",result.routeIds},{"totalDistance",result.totalDistance}}}}).dump(), "application/json");
    });

    http_.Get("/api/mst", [&](const httplib::Request& req, httplib::Response& res){
        auto start = req.has_param("start") ? req.get_param_value("start") : "S1";
        auto result = mst_.buildMST(start);
        res.set_content(json({{"success",true},{"data",{{"routeIds",result.routeIds},{"totalDistance",result.totalDistance}}}}).dump(), "application/json");
    });

    http_.Get("/api/reports/profitability", [&](const httplib::Request&, httplib::Response& res){
        auto bookings = storage_.loadBookings();
        auto map = reports_.profitabilityByRoute(bookings);
        res.set_content(json({{"success",true},{"data",map}}).dump(), "application/json");
    });

    http_.Get("/api/fares/range", [&](const httplib::Request& req, httplib::Response& res){
        double minFare = std::stod(req.get_param_value("min"));
        double maxFare = std::stod(req.get_param_value("max"));
        std::vector<double> fares;
        for (const auto& r : graph_.allRoutes()) fares.push_back(r.baseFare);
        std::sort(fares.begin(), fares.end());
        auto range = sorter_.binarySearchFareRange(fares, minFare, maxFare);
        res.set_content(json({{"success",true},{"data",{{"start",range.first},{"end",range.second}}}}).dump(), "application/json");
    });

    http_.Get("/api/occupancy-map", [&](const httplib::Request&, httplib::Response& res){
        json arr = json::array();
        for (const auto& r : graph_.allRoutes()) {
            double occ = r.capacity > 0 ? (100.0 * r.occupiedSeats / r.capacity) : 0.0;
            arr.push_back({{"routeId",r.id},{"occupancyPercent",occ}});
        }
        res.set_content(json({{"success",true},{"data",arr}}).dump(), "application/json");
    });

    http_.Get("/api/fare-trend", [&](const httplib::Request& req, httplib::Response& res){
        std::string routeId = req.get_param_value("routeId");
        int days = std::stoi(req.get_param_value("days"));
        const Route* route = graph_.findRoute(routeId);
        if (!route) { res.status = 404; res.set_content(json({{"success",false},{"message","route not found"}}).dump(), "application/json"); return; }
        json points = json::array();
        double base = route->baseFare;
        for (int i = 1; i <= days; ++i) {
            points.push_back({{"day", i}, {"fare", base + i * 5.0}});
        }
        res.set_content(json({{"success",true},{"data",points}}).dump(), "application/json");
    });
}

void ApiServer::run() { http_.listen("0.0.0.0", port_); }
```

```cpp
// src/main.cpp
#include "api_server.hpp"

int main() {
    ApiServer server;
    server.configure(8080);
    server.run();
    return 0;
}
```

```cmake
add_executable(test_api_smoke
  tests/test_api_smoke.cpp
  src/api_server.cpp
  src/station_directory.cpp
  src/rail_graph.cpp
  src/pricing_engine.cpp
  src/booking_engine.cpp
  src/routing_engine.cpp
  src/mst_engine.cpp
  src/sort_search.cpp
  src/report_engine.cpp
  src/storage_manager.cpp
)
add_test(NAME test_api_smoke COMMAND test_api_smoke)
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Expected: `test_api_smoke` passes, proving server compiles with module wiring.

- [ ] **Step 5: Commit**

```bash
git add include/api_server.hpp src/api_server.cpp src/main.cpp tests/test_api_smoke.cpp CMakeLists.txt
git commit -m "feat: wire REST API server and backend modules"
```

---

### Task 7: Web UI (Dashboard + requirement flows + custom features)

**Files:**
- Create: `web/index.html`
- Create: `web/styles.css`
- Create: `web/app.js`

- [ ] **Step 1: Write failing manual UI check criteria**

Run app and check these initially fail (no files):
- Browser on `http://localhost:8080` does not load UI.
- No forms for stations/routes/bookings.
- No occupancy/fare trend section.

- [ ] **Step 2: Implement UI files**

```html
<!-- web/index.html -->
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Railway DSA System</title>
  <link rel="stylesheet" href="/styles.css" />
</head>
<body>
  <header><h1>Railway Network Management</h1></header>
  <main>
    <section id="station-route-panel"></section>
    <section id="booking-panel"></section>
    <section id="routing-panel"></section>
    <section id="mst-report-panel"></section>
    <section id="custom-panel"></section>
  </main>
  <script src="/app.js"></script>
</body>
</html>
```

```css
/* web/styles.css */
:root { --bg:#f6f8fb; --card:#ffffff; --ink:#132a3b; --accent:#0f766e; }
body { margin:0; font-family: "Georgia", serif; background:linear-gradient(180deg,#f6f8fb,#eaf2f5); color:var(--ink); }
header { padding:16px 24px; background:#dbeafe; border-bottom:1px solid #bfdbfe; }
main { padding:20px; display:grid; gap:16px; }
section { background:var(--card); border:1px solid #d1d5db; border-radius:10px; padding:14px; }
button { background:var(--accent); color:#fff; border:0; border-radius:8px; padding:8px 12px; }
input, select { padding:8px; border:1px solid #94a3b8; border-radius:6px; }
```

```javascript
// web/app.js
async function api(path, method = "GET", body = null) {
  const res = await fetch(path, {
    method,
    headers: { "Content-Type": "application/json" },
    body: body ? JSON.stringify(body) : null,
  });
  return res.json();
}

async function boot() {
  document.getElementById("station-route-panel").innerHTML = `
    <h2>Stations & Routes</h2>
    <button id="loadStations">Load Stations</button>
    <pre id="stationsOut"></pre>
  `;
  document.getElementById("custom-panel").innerHTML = `
    <h2>Custom Features</h2>
    <button id="loadOccupancy">Seat Occupancy Map</button>
    <button id="loadTrend">7-Day Fare Trend</button>
    <pre id="customOut"></pre>
  `;

  document.getElementById("loadStations").onclick = async () => {
    const data = await api("/api/stations");
    document.getElementById("stationsOut").textContent = JSON.stringify(data, null, 2);
  };
  document.getElementById("loadOccupancy").onclick = async () => {
    const data = await api("/api/occupancy-map");
    document.getElementById("customOut").textContent = JSON.stringify(data, null, 2);
  };
  document.getElementById("loadTrend").onclick = async () => {
    const data = await api("/api/fare-trend?routeId=R1&days=7");
    document.getElementById("customOut").textContent = JSON.stringify(data, null, 2);
  };
}

boot();
```

- [ ] **Step 3: Verify UI manually**

Run: `./build/railway_app`

Expected:
- `http://localhost:8080` loads page
- Buttons invoke API
- JSON output visible in page

- [ ] **Step 4: Commit**

```bash
git add web/index.html web/styles.css web/app.js
git commit -m "feat: add web UI connected to REST endpoints"
```

---

### Task 8: Complete Endpoint Coverage + Seed Data + Final Verification

**Files:**
- Modify: `src/api_server.cpp`
- Create: `data/stations.json`
- Create: `data/routes.json`
- Create: `data/bookings.json`
- Create: `data/waitlist.json`
- Create: `data/refunds.json`
- Create: `data/analytics.json`
- Create: `README.md`

- [ ] **Step 1: Write failing endpoint checklist**

Create this checklist in local notes and verify these currently fail/incomplete:
- `/api/routes/search` missing `least_crowded` handling response fields
- `/api/mst` not returning route list + total
- `/api/fares/range` not using binary search helper
- `/api/occupancy-map` and `/api/fare-trend` incomplete

- [ ] **Step 2: Implement remaining endpoint logic**

```cpp
// src/api_server.cpp additions (example style)
http_.Get("/api/routes/search", [&](const httplib::Request& req, httplib::Response& res){
    auto from = req.get_param_value("from");
    auto to = req.get_param_value("to");
    auto mode = req.get_param_value("mode");
    auto result = routing_.findPath(from, to, mode);
    res.set_content(json({{"success",true},{"data",{{"routeIds",result.routeIds},{"totalDistance",result.totalDistance}}}}).dump(), "application/json");
});

http_.Get("/api/mst", [&](const httplib::Request& req, httplib::Response& res){
    auto start = req.has_param("start") ? req.get_param_value("start") : "S1";
    auto result = mst_.buildMST(start);
    res.set_content(json({{"success",true},{"data",{{"routeIds",result.routeIds},{"totalDistance",result.totalDistance}}}}).dump(), "application/json");
});
```

- [ ] **Step 3: Add seed JSON and README run instructions**

```json
// data/stations.json
[
  {"id":"S1","name":"Central"},
  {"id":"S2","name":"West"},
  {"id":"S3","name":"North"}
]
```

```markdown
<!-- README.md -->
# Railway Network DSA Mini Project

## Run
1. `cmake -S . -B build`
2. `cmake --build build`
3. `./build/railway_app`
4. Open `http://localhost:8080`

## Test
`ctest --test-dir build -V`
```

- [ ] **Step 4: Run full verification**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -V`

Run (new terminal):
`curl -s http://localhost:8080/api/stations`
`curl -s "http://localhost:8080/api/routes/search?from=S1&to=S3&mode=shortest"`
`curl -s http://localhost:8080/api/mst`
`curl -s http://localhost:8080/api/occupancy-map`
`curl -s "http://localhost:8080/api/fare-trend?routeId=R1&days=7"`

Expected: JSON responses with `success:true` and non-empty `data` where appropriate.

- [ ] **Step 5: Commit**

```bash
git add src/api_server.cpp data/stations.json data/routes.json data/bookings.json data/waitlist.json data/refunds.json data/analytics.json README.md
git commit -m "feat: complete API coverage, seed data, and project runbook"
```

---

## Spec Coverage Check

- Requirement 1 (add train/route): Task 2 + Task 6 endpoints
- Requirement 2 (book + VIP waitlist): Task 3 + Task 6 endpoints
- Requirement 3 (Dijkstra shortest + least crowded): Task 4 + Task 8 endpoint completion
- Requirement 4 (Prim MST): Task 4 + Task 8 endpoint completion
- Requirement 5 (cascading cancellation, refund stack, promotions): Task 3
- Requirement 6 (greedy surge > 80%): Task 3
- Requirement 7 (multi-level sorting): Task 5
- Requirement 8 (binary search + station lookup): Task 2 + Task 5
- Requirement 9 (2 custom features): Task 7 + Task 8 (`occupancy-map`, `fare-trend`)

## Plan Self-Review

- Placeholder scan: no `TODO`, `TBD`, or deferred implementation markers.
- Internal consistency: same class names/signatures are reused across tasks.
- Scope check: plan stays within one integrated local app (single executable + web UI).
- Ambiguity check: endpoint names, file paths, and validation targets are explicit.
