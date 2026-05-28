# Railway Network Management System (DSA Mini Project) - Design

Date: 2026-05-17

## Goals
- Build a macOS SwiftUI app for a Railway Network Management System.
- Implement all core features listed in the provided spec image (items 1-8).
- Add two custom features: real-time delay simulation and admin analytics dashboard.
- Use C++ for all algorithms and data structures; SwiftUI is UI-only.
- Persist data between runs using simple JSON files.

## Non-Goals
- No cloud sync, accounts, or multi-user authentication.
- No heavy UI theming or animation beyond basic SwiftUI polish.
- No performance tuning beyond what is needed for a demo-scale dataset.

## Architecture

### Overview
- **SwiftUI UI layer**: macOS app with structured navigation and forms.
- **Objective-C++ bridge**: a thin wrapper to call C++ from Swift.
- **C++ core**: data structures + algorithms + persistence.

### Layer Responsibilities
- **SwiftUI**: input validation, view state, user flows.
- **Bridge**: type conversion and error mapping.
- **C++ core**: business logic, algorithms, persistence.

## Components

### C++ Core
- **StationStore**: station records in array/linked list; optional sorted index for binary search.
- **Graph**: adjacency list with `RouteEdge` storing distance, fare, capacity, occupancy, delay.
- **RoutingEngine**: modified Dijkstra for shortest + least crowded; supports weight tuning.
- **MSTEngine**: Prim's algorithm for cost-optimized network report.
- **PricingEngine**: surge logic when occupancy > 80%.
- **BookingEngine**: seat booking, VIP-first waitlist queue, refunds stack, auto-promotions.
- **Analytics**: route profitability, top routes, cancellations, occupancy stats.
- **Persistence**: JSON save/load for stations, routes, bookings, waitlist, analytics counters.

### Bridge (Objective-C++)
- `CoreBridge` exposes Swift-friendly methods:
  - `addStation`, `addRoute`, `removeRoute`
  - `bookSeat`, `cancelBooking`, `getWaitlist`
  - `findRoutes`, `getMSTReport`, `getAnalytics`
  - `setDelay`, `save`, `load`

### SwiftUI UI
- **Stations/Routes**: add/edit routes and stations.
- **Booking**: seat booking + waitlist view.
- **Routing**: search + ranked route results.
- **Analytics**: dashboards and MST report.
- **Admin/Config**: delay simulation, pricing thresholds.

## Data Flow
- **Add station/route**: SwiftUI -> Bridge -> StationStore/Graph -> UI refresh.
- **Book seat**: SwiftUI -> BookingEngine (pricing + seat allocation) -> UI.
- **Cancel**: SwiftUI -> BookingEngine -> refunds stack + waitlist promotion -> UI.
- **Routing search**: SwiftUI -> RoutingEngine (delay + crowd weights) -> UI list.
- **MST report**: SwiftUI -> MSTEngine -> UI.
- **Analytics**: SwiftUI -> Analytics -> UI cards.
- **Persistence**: save/load via Persistence module.

## Algorithms and Data Structures
- **Dijkstra (modified)**: distance + crowd weight + delay weight.
- **Prim's MST**: cost-optimized network analysis.
- **Greedy pricing**: surge when occupancy > 80%.
- **Sorting**: multi-level sort on route -> date -> fare.
- **Linked List/Array**: station lookup + index (binary search on sorted list).
- **Queue**: waitlist (VIP before regular).
- **Stack**: refunds history.

## Error Handling
- Validate all inputs (non-empty, positive values).
- Bridge returns structured errors (station not found, route exists, no path).
- Core uses error codes or `Result` objects to avoid throwing across bridge.
- Save/load reports clear failures.

## Persistence
- JSON files stored in app support directory.
- On app launch: attempt load, otherwise start empty.
- On app close or manual save: write JSON.

## Testing
- C++ unit tests: graph operations, Dijkstra, Prim, pricing, waitlist/stack, persistence.
- SwiftUI smoke test: verify flows add route -> book -> cancel -> reroute -> report.
- Integration tests: bridge calls for core flows.

## Milestones
1. C++ core models + algorithms.
2. Objective-C++ bridge.
3. SwiftUI scaffolding and forms.
4. Integration and persistence.
5. Testing and polish.
