# P2: Waitlist Queue + Cancellation Refund Stack

## Your Role
You are responsible for the **waitlist Queue** and the **cancellation refund Stack**. You own the entire booking lifecycle — confirming seats, managing VIP-priority FIFO waitlists, handling cancellations with LIFO refund tracking, and auto-promoting waitlisted passengers when seats free up.

## Files You Own

| File | Lines | What it does |
|------|-------|--------------|
| `include/booking_engine.hpp` + `src/booking_engine.cpp` | ~120 | Complete booking system: book, cancel, waitlist, refund, promotion |

You also depend on:
- `include/models.hpp` — `Booking`, `BookingStatus`, `Route` records (owned by P1)
- `include/rail_graph.hpp` — to check/update seat counts on routes (owned by P1)

---

## 1. The Three Core Operations

### Book a Seat
```cpp
Booking BookingEngine::bookSeat(const std::string& routeId,
                                 const std::string& passengerName, bool isVip) {
  Booking booking{nextBookingId(), routeId, passengerName, isVip,
                  BookingStatus::Waitlisted};  // always starts as Waitlisted

  if (confirmSeat(routeId)) {
    booking.status = BookingStatus::Confirmed;  // upgraded if seats available
  } else {
    addToWaitlist(booking);  // enqueued if full
  }

  bookings_.push_back(booking);
  return booking;
}
```

**Flow**: Start as Waitlisted → try Confirm → if no space → enqueue to waitlist → return with final status. The booking is always stored in `bookings_` regardless of status.

### Cancel a Booking
```cpp
bool BookingEngine::cancelBooking(const std::string& bookingId) {
  // find booking in bookings_ vector
  auto it = std::find_if(bookings_.begin(), bookings_.end(),
      [&](const Booking& b) { return b.id == bookingId; });
  if (it == bookings_.end()) return false;

  Booking& booking = *it;
  if (booking.status == BookingStatus::Cancelled) return false;  // idempotent

  BookingStatus previousStatus = booking.status;
  booking.status = BookingStatus::Cancelled;

  refundStack_.push_back(bookingId);  // PUSH onto STACK (LIFO)

  if (previousStatus == BookingStatus::Confirmed) {
    // free a seat on the route
    const Route* route = graph_.findRoute(booking.routeId);
    graph_.updateOccupiedSeats(booking.routeId, route->occupiedSeats - 1);
    promoteWaitlisted(booking.routeId);  // auto-promote next in queue!
  }
  return true;
}
```

**Flow**: Find booking → mark Cancelled → push to refund stack → if it was Confirmed, free seat + auto-promote from waitlist.

### Waitlist Promotion (Auto-Promotion)
```cpp
bool BookingEngine::promoteWaitlisted(const std::string& routeId) {
  // VIPs FIRST — check VIP queue before regular
  if (!waitlistVip_.empty()) {
    Booking booking = waitlistVip_.front();
    waitlistVip_.pop_front();           // DEQUEUE (FIFO)
    if (confirmSeat(routeId)) {
      // mark booking.status = Confirmed in bookings_
      return true;
    }
    return false;
  }

  // Then regular passengers
  if (!waitlistRegular_.empty()) {
    Booking booking = waitlistRegular_.front();
    waitlistRegular_.pop_front();       // DEQUEUE (FIFO)
    if (confirmSeat(routeId)) {
      // mark booking.status = Confirmed in bookings_
      return true;
    }
  }
  return false;
}
```

---

## 2. DSA Concepts: Queue & Stack

### Queue (FIFO) — Waitlist

```
Back (enqueue here)     Front (dequeue from here)
    ↓                       ↓
  [Reg3] ← [Reg2] ← [Reg1]
```
```cpp
std::deque<Booking> waitlistVip_;     // FIFO for VIPs
std::deque<Booking> waitlistRegular_; // FIFO for regular passengers

// Enqueue (add to back)
waitlistVip_.push_back(booking);      // O(1)

// Dequeue (remove from front)
Booking next = waitlistVip_.front();  // peek at oldest
waitlistVip_.pop_front();             // remove oldest (O(1))
```

**Why FIFO?** Fairness — the first person to join the waitlist should be the first person to get a seat when one frees up.

### Stack (LIFO) — Refund History

```
       Push (B3) → [B1, B2, B3] ← Top (most recent)
       Pop  (B3) ← [B1, B2]
```
```cpp
std::vector<std::string> refundStack_;  // used as a stack

// Push
refundStack_.push_back(bookingId);      // O(1) amortized

// Peek (most recent cancellation)
refundStack_.back();                    // O(1)

// Pop (if we needed it)
refundStack_.pop_back();                // O(1)
```

**Why LIFO?** The most recent cancellation is the most immediately relevant. No "first cancellation" priority needed — we just track history for potential reporting/undo.

### Container Choices — Know These!

| Aspect | Waitlist (Queue) | Refund (Stack) |
|--------|-----------------|----------------|
| Abstract type | FIFO Queue | LIFO Stack |
| Actual container | `std::deque<Booking>` | `std::vector<std::string>` |
| Why not adaptor? | `std::queue` wraps deque anyway | `std::stack` wraps deque anyway |
| Why this choice? | Deque allows random access for debugging | Vector is simpler, inspectable in tests |

### `std::deque` vs `std::list` vs `std::queue`

| Property | `std::deque` | `std::list` | `std::queue` (adaptor) |
|----------|-------------|-------------|----------------------|
| Underlying | Array of blocks | Doubly linked nodes | Defaults to deque |
| `push_back` | O(1) | O(1) | O(1) |
| `pop_front` | O(1) | O(1) | O(1) |
| `[]` access | O(1) | O(N) | No |
| Memory | Contiguous blocks | Fragmented nodes | Same as deque |
| Iterator invalidation | push_front/back may invalidate | never invalidates | Same as deque |

**Why deque over list?** Deque's block-based memory is more cache-friendly than list's per-node allocations. Deque also gives O(1) random access if needed for debugging.

---

## 3. VIP Priority Logic

```cpp
void addToWaitlist(const Booking& booking) {
  if (booking.isVip) waitlistVip_.push_back(booking);       // separate VIP queue
  else waitlistRegular_.push_back(booking);                  // separate regular queue
}

bool promoteWaitlisted(const std::string& routeId) {
  // VIPs have absolute priority
  if (!waitlistVip_.empty()) {
    promote from waitlistVip_.front();  // oldest VIP first
    return true;
  }
  // Regular only if no VIPs waiting
  if (!waitlistRegular_.empty()) {
    promote from waitlistRegular_.front();  // oldest regular first
  }
}
```

**Two-Queue Model**: Separate queues for VIP and regular. VIPs always served first. Within each queue, strict FIFO. This is a **priority queue pattern** implemented with two regular queues.

---

## 4. Booking ID Generation

```cpp
int bookingCounter_ = 0;

std::string BookingEngine::nextBookingId() {
  bookingCounter_ += 1;
  return "B" + std::to_string(bookingCounter_);
}
```

Sequential: B1, B2, B3... Simple, deterministic, easy to read.

---

## 5. Seat Confirmation

```cpp
bool BookingEngine::confirmSeat(const std::string& routeId) {
  const Route* route = graph_.findRoute(routeId);
  if (!route) return false;                        // route doesn't exist
  if (route->availableSeats() <= 0) return false;  // train is full
  return graph_.updateOccupiedSeats(routeId, route->occupiedSeats + 1);
}
```

---

## 6. Real-World Analogy

- **Queue (Waitlist)**: Standing in line at a ticket counter. The person who arrived first gets served first. VIPs have a separate, faster line.
- **Stack (Refund)**: A pile of cancellation slips. The most recent one is on top.

---

## Common Viva Questions

**Q: Why is waitlist a queue and refund a stack?**
- **Queue**: First-come, first-served is fair. The person waiting longest should get the next seat.
- **Stack**: We just need to record cancellation history. Most recent cancellation is most relevant. No ordering priority needed.

**Q: Walk through booking when seats are available.**
1. `bookSeat("R1", "Alice", false)` called
2. Booking created with status `Waitlisted`
3. `confirmSeat("R1")` → `availableSeats() > 0` → `occupiedSeats++` → returns true
4. Booking upgraded to `Confirmed`
5. Booking pushed to `bookings_`, returned to caller

**Q: Walk through booking when no seats are available.**
1. `bookSeat("R1", "Bob", false)` called
2. Booking created with status `Waitlisted`
3. `confirmSeat("R1")` → `availableSeats() <= 0` → returns false
4. `addToWaitlist(bobBooking)` → pushed to `waitlistRegular_` back
5. Booking stays `Waitlisted`, returned to caller

**Q: Walk through cancellation with people waiting.**
1. `cancelBooking("B1")` called (B1 is Confirmed)
2. B1 status → `Cancelled`
3. B1's ID pushed to `refundStack_`
4. Since B1 was Confirmed: `occupiedSeats` decremented on that route
5. `promoteWaitlisted("R1")` called
6. VIP queue checked first → if VIP, promote. Else check regular queue
7. Front of queue gets `Confirmed`, status updated, dequeued

**Q: What if there are no waitlisted passengers?**
`promoteWaitlisted` checks both queues. Both empty → returns false. No-op.

**Q: Can the same booking be cancelled twice?**
No. `if (booking.status == Cancelled) return false;` — idempotent, no error thrown.

**Q: What's the time complexity of each operation?**
| Operation | Complexity | Why |
|-----------|-----------|-----|
| `bookSeat` | O(1) | No search on route ID (passed directly) |
| `cancelBooking` | O(N) | Linear search for booking by ID in bookings_ |
| `promoteWaitlisted` | O(1) | Front/pop_front on deque |
| `addToWaitlist` | O(1) | push_back on deque |

**Q: What if a VIP and regular are both waiting — who gets the seat?**
Always the VIP. `promoteWaitlisted` checks `waitlistVip_` first. Regular queue is only checked if VIP queue is empty.

**Q: Why two queues instead of one sorted queue?**
Simpler and clearer. Two deques with a priority check in `promoteWaitlisted` is easier to understand and maintain than a single priority queue with comparison logic.

**Q: What is `std::deque` internally?**
A **deque** (double-ended queue) is implemented as a sequence of fixed-size blocks (arrays). It grows by adding new blocks, not by reallocating a single contiguous array like vector. This is why `push_front` and `pop_front` are O(1).

**Q: How would you add a "undo last cancellation" feature?**
Pop the refund stack: `refundStack_.back()` gives the last cancelled booking ID. Find that booking, set status back to Confirmed, increment occupiedSeats. O(1) for the stack pop + O(N) to find the booking.

**Q: What real-world system inspired this design?**
Indian Railways' waitlist system. They have a queue of waitlisted passengers (RLWL, PQWL) and auto-cancellation/confirmation when seats free up. VIP priority mirrors their "VIP quota" system.
