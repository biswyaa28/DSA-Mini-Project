# Viva & Presentation Prep — Railway Network Management System

## PROJECT OVERVIEW (30-sec pitch)

> "A C++ REST API + web UI that manages a railway network. It demonstrates 8+ DSA concepts: **graphs** for stations/routes, **Dijkstra's** for shortest path, **Prim's** for minimum spanning tree, **queues** for waitlist, **stacks** for refunds, **sorting** and **binary search** for fare analysis, and **greedy algorithms** for surge pricing."

---

## PRESENTATION FLOW (5-7 min)

### 1. Problem Statement (30s)
"Manage a railway network — stations, routes, bookings, cancellations. Need shortest path routing, fair waitlists, and pricing logic."

### 2. Architecture (1 min)
- C++ backend (REST API on port 8080)
- Web frontend (HTML/CSS/JS with vis.js graph visualization)
- Seed data: 5 stations, 7 routes connected
- **Show the network graph** (open `http://localhost:8080`)

### 3. Data Structures — Arrays/Vectors (1 min)
- `StationDirectory` — `std::vector<Station>`, linear search O(N)
- `RailGraph` — edge list as `std::vector<Route>`, outgoing queries scan all edges
- Why vector? Cache-friendly, O(1) amortized push_back, simple

### 4. Queue + Stack — Booking System (1 min)
- **Queue (FIFO)**: VIP-first waitlist using `std::deque` — fair, first-come-first-served
- **Stack (LIFO)**: Refund history — most recent cancellation is most relevant
- Auto-promotion: when a seat frees up, VIP queue is checked first

### 5. Algorithms — Dijkstra + Prim's (1.5 min)
- **Dijkstra**: Two modes — `shortest` (raw distance) and `least_crowded` (distance × occupancy penalty)
- **Prim's**: Builds minimum-cost spanning tree from a start station
- Both O(V²) — fine for 5 stations, would use priority queue at scale

### 6. Greedy + Sorting + Search (1 min)
- **Surge pricing**: 1.3× when occupancy > 80% — greedy, local decision
- **Multi-level sort**: routeId > travelDate > fare using `std::sort` (Introsort)
- **Binary search**: Two binary searches for fare range [min, max] in O(log N)

### 7. Demo (optional, 1-2 min)
Show: booking → waitlist → cancel → auto-promotion, or shortest-path between stations

---

## KEY CODE TO REFERENCE (memorize these)

### Dijkstra's edgeWeight — the key innovation
```cpp
double edgeWeight(const Route& r, const std::string& mode) const {
  if (mode == "shortest") return r.distanceKm;
  double occ = r.occupiedSeats / r.capacity;
  return r.distanceKm * (1.0 + occ);  // penalty for crowded routes
}
```

### Waitlist — queue operations
```cpp
// Enqueue
if (booking.isVip) waitlistVip_.push_back(booking);
else waitlistRegular_.push_back(booking);

// Dequeue (promote)
Booking b = waitlistVip_.front();  // peek
waitlistVip_.pop_front();          // remove
```

### Surge check — integer math (no floats!)
```cpp
return occupiedSeats * 100 > capacity * 80;
```

### Binary search — overflow-safe mid
```cpp
int mid = lo + (hi - lo) / 2;  // prevents (lo+hi) overflow
```

---

## 20 LIKELY VIVA QUESTIONS

### Q1: What's the project about?
> A railway network management system with routing, booking, waitlists, cancellations, reports, and dynamic pricing — demonstrating arrays, graphs, Dijkstra, Prim's, queues, stacks, sorting, binary search, and greedy algorithms.

### Q2: Why std::vector over linked list?
> Vector is cache-friendly (contiguous memory), O(1) amortized append, O(1) random access. Linked list (std::list) only wins for frequent middle insertions/deletions — which we don't do.

### Q3: How is waitlist different from refund?
> Waitlist = **Queue (FIFO)** — fairness, longest-waiting person gets seat first. Refund = **Stack (LIFO)** — most recent cancellation tracked for reporting, no priority needed.

### Q4: Why is there a VIP queue and regular queue?
> VIPs get priority — all VIP waitlisted passengers are served before any regular passenger. Within each queue it's FIFO.

### Q5: Explain Dijkstra's algorithm flow
> 1. Set source distance = 0, all others = ∞. 2. Pick unvisited node with smallest distance. 3. Relax all its outgoing edges. 4. Mark visited. 5. Repeat until destination reached. O(V²) in our implementation.

### Q6: What's "least crowded" mode?
> Edge weight = distance × (1 + occupancyRatio). A full train gets double the effective distance, so Dijkstra avoids it. Same algorithm, different weight function.

### Q7: Dijkstra vs Prim's difference?
> Both grow from a source. Dijkstra finds shortest path TO each node from source (minimizes source-to-node distance). Prim's connects ALL nodes minimizing total edge weight (spanning tree). Dijkstra can stop early; Prim's must visit all.

### Q8: How does Prim's avoid cycles?
> Track visited nodes in `inMst` set. Only add edges to UNVISITED destinations. Never pick an edge whose destination is already in the MST.

### Q9: Is surge pricing really greedy?
> Yes — it makes the locally optimal decision (raise price NOW if train is >80% full) without considering future demand or patterns. Greedy = immediate best choice.

### Q10: What algorithm does std::sort use?
> **Introsort** — hybrid of Quicksort + Heapsort + Insertion Sort. Starts with Quicksort, switches to Heapsort if recursion depth exceeds log N (prevents O(N²) worst case), uses Insertion Sort for small partitions.

### Q11: Why two binary searches for fare range?
> First finds lower bound (first index ≥ minFare), second finds upper bound (last index ≤ maxFare). Two O(log N) searches = still O(log N). Linear scan after first hit would be O(N).

### Q12: Why `(hi - lo) / 2` instead of `(lo + hi) / 2`?
> Prevents integer overflow when lo and hi are both large.

### Q13: std::map vs std::unordered_map?
> `map` = Red-Black Tree (sorted, O(log N)), `unordered_map` = hash table (unsorted, O(1) average). Used `map` for profitability so routes come out alphabetically sorted.

### Q14: Why struct not class?
> Plain data containers — all fields public. No encapsulation needed. Getter/setter boilerplate would add nothing.

### Q15: How to scale to 10,000 stations?
> 1. Replace linear search with `unordered_map` for O(1) lookups. 2. Build adjacency list instead of edge list scan. 3. Dijkstra with priority queue for O((V+E) log V).

### Q16: What if no path exists (Dijkstra)?
> `if (!dist.count(dst)) return PathResult{}` — empty result with 0 distance. API returns 404.

### Q17: Can you cancel twice?
> No — `cancelBooking` checks `if (status == Cancelled) return false`.

### Q18: What happens when a confirmed booking is cancelled with people waiting?
> 1. Mark booking cancelled. 2. Push to refund stack. 3. Decrement occupiedSeats. 4. `promoteWaitlisted()` — VIP queue first, then regular. First waitlisted person gets confirmed.

### Q19: How does the fare trend predictor work?
> Simulates linear occupancy growth over N days. At each step checks if >80% occupancy would trigger surge. Returns projected fare per day. It's a simple predictive model, not real data.

### Q20: What would you improve?
> 1. Priority queue for Dijkstra (O(V²)→O(E log V)). 2. `unordered_map` for station lookups (O(N)→O(1)). 3. Adjacency list instead of edge list scan (O(E)→O(1) per outgoing query). 4. More test coverage for edge cases.

---

## QUICK REFERENCE CARD

| DSA Concept | Where | Complexity |
|---|---|---|
| Dynamic Array (vector) | Stations, routes, bookings | O(1) append, O(N) search |
| Queue (deque) | VIP/Regular waitlist | O(1) enqueue/dequeue |
| Stack (vector) | Refund history | O(1) push/pop |
| Graph (edge list) | RailGraph | O(E) outgoing |
| Dijkstra | RoutingEngine | O(V²) |
| Prim's MST | MSTEngine | O(V² × E) |
| Introsort | std::sort | O(N log N) |
| Binary search | Fare range | O(log N) |
| Red-Black Tree | std::map (profitability) | O(log N) per op |
| Greedy | Surge pricing | O(1) |

### Seed Network (memorize!)
```
S1(Central) ─R1(10km)─▶ S2(West) ─R2(15km)─▶ S3(North)
S1 ──────────R3(40km)─────────────────────────▶ S3
S3 ──────────R4(20km)─────────────────────────▶ S4(East)
S2 ──────────R5(25km)─────────────────────────▶ S4
S1 ─R6(30km)─▶ S5(South) ─R7(10km)─▶ S4
```

### Run commands
```bash
./run.sh                          # Build + run
open http://localhost:8080        # Open UI
ctest --test-dir build -V         # Run all tests
```
