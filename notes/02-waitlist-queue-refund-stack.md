# Viva Prep — P2: Waitlist Queue + Cancellation Refund Stack

## Your Role
You handle the booking system — confirming seats, managing a VIP-priority waitlist (queue), and tracking cancellations with a refund stack.

## Key File You Own

| File | Lines | What it does |
|------|-------|-------------|
| `include/booking_engine.hpp` + `src/booking_engine.cpp` | 120 | The entire booking + waitlist + refund system |

You also depend on:
- `include/models.hpp` — `Booking`, `BookingStatus`, `Route` structs
- `include/rail_graph.hpp` — to check/update seat counts

---

## 1. The Three Core Operations

### Book a Seat
```cpp
Booking BookingEngine::bookSeat(const std::string& routeId,
                                 const std::string& passengerName, bool isVip) {
  Booking booking{nextBookingId(), routeId, passengerName, isVip,
                  BookingStatus::Waitlisted};  // starts as Waitlisted

  if (confirmSeat(routeId)) {
    booking.status = BookingStatus::Confirmed;  // upgraded if space
  } else {
    addToWaitlist(booking);  // enqueued if full
  }

  bookings_.push_back(booking);
  return booking;
}
```

**Flow**: Try to confirm → if no space → add to waitlist → return booking with whatever status it got.

### Cancel a Booking
```cpp
bool BookingEngine::cancelBooking(const std::string& bookingId) {
  // find booking
  if (booking.status == BookingStatus::Cancelled) return false;  // can't cancel twice

  booking.status = BookingStatus::Cancelled;
  refundStack_.push_back(bookingId);  // push onto STACK (LIFO)

  if (previousStatus == BookingStatus::Confirmed) {
    // free a seat on the route
    graph_.updateOccupiedSeats(routeId, route->occupiedSeats - 1);
    promoteWaitlisted(routeId);  // auto-promote the next person!
  }
  return true;
}
```

### Waitlist Promotion (Auto-Promotion)
```cpp
bool BookingEngine::promoteWaitlisted(const std::string& routeId) {
  // VIPs first!
  if (!waitlistVip_.empty()) {
    Booking booking = waitlistVip_.front();
    waitlistVip_.pop_front();  // DEQUEUE
    if (confirmSeat(routeId)) {
      // mark as Confirmed
      return true;
    }
    return false;
  }

  // Then regular passengers
  if (!waitlistRegular_.empty()) {
    Booking booking = waitlistRegular_.front();
    waitlistRegular_.pop_front();  // DEQUEUE
    if (confirmSeat(routeId)) {
      // mark as Confirmed
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
Enqueue → [VIP1, VIP2, ...] → Dequeue (front)
Enqueue → [Reg1, Reg2, ...] → Dequeue (front)
```

```cpp
std::deque<Booking> waitlistVip_;     // FIFO queue for VIPs
std::deque<Booking> waitlistRegular_; // FIFO queue for regular

void addToWaitlist(const Booking& booking) {
  if (booking.isVip) waitlistVip_.push_back(booking);   // enqueue at back
  else waitlistRegular_.push_back(booking);
}

// When a seat frees up:
waitlistVip_.front();   // peek at front (oldest waiting VIP)
waitlistVip_.pop_front(); // dequeue
```

**Why FIFO?** Fairness — the person who's been waiting longest gets the seat first.

### Stack (LIFO) — Refund History

```
Push → [B1, B2, ...] ← Top/Pop
```

```cpp
std::vector<std::string> refundStack_;  // used as a stack

// When cancelling:
refundStack_.push_back(bookingId);  // PUSH onto stack

// To check last refund:
refundStack_.back();   // PEEK at top
```

**Why LIFO?** The most recent cancellation is the most relevant one to check. No "first refund" priority needed.

### Why `std::deque` instead of `std::queue`?
- `std::queue` is a container adaptor that defaults to `std::deque` internally anyway
- `std::deque` supports both `push_back`/`pop_front` (queue ops) AND random access
- Deque also allows `[]` indexing if needed for debugging

### Why `std::vector` instead of `std::stack` for refunds?
- Same reason — `std::stack` defaults to `std::deque` internally
- `std::vector` with `push_back`/`back`/`pop_back` works identically
- Easier to inspect in unit tests

---

## 3. VIP Priority Logic

```cpp
void addToWaitlist(const Booking& booking) {
  if (booking.isVip) waitlistVip_.push_back(booking);       // VIP queue
  else waitlistRegular_.push_back(booking);                   // regular queue
}

bool promoteWaitlisted(const std::string& routeId) {
  if (!waitlistVip_.empty()) {
    // promote from VIP queue FIRST
    waitlistVip_.front(); waitlistVip_.pop_front();
  }
  if (!waitlistRegular_.empty()) {
    // promote from regular queue only if VIP queue is empty
    waitlistRegular_.front(); waitlistRegular_.pop_front();
  }
}
```

**All VIPs are served before any regular passenger.** Within each queue, it's FIFO.

---

## 4. Booking ID Generation (Auto-Increment)

```cpp
int bookingCounter_ = 0;

std::string BookingEngine::nextBookingId() {
  bookingCounter_ += 1;
  return "B" + std::to_string(bookingCounter_);
}
```

Sequential: B1, B2, B3...

---

## 5. Seat Confirmation (Checks Capacity)

```cpp
bool BookingEngine::confirmSeat(const std::string& routeId) {
  const Route* route = graph_.findRoute(routeId);
  if (!route) return false;               // route doesn't exist
  if (route->availableSeats() <= 0) return false;  // full
  return graph_.updateOccupiedSeats(routeId, route->occupiedSeats + 1);  // book it
}
```

---

## Common Viva Questions

**Q: Why is waitlist a queue and refund a stack?**
- **Queue**: First-come, first-served is fair. The person who has been waiting longest should get the next available seat.
- **Stack**: Last cancellation is most recent. No priority needed — just tracking the history for reporting.

**Q: What happens step-by-step when booking with no seats left?**
1. `bookSeat("R1", "Alice", false)` called
2. `confirmSeat("R1")` sees 0 available seats → returns false
3. `addToWaitlist(aliceBooking)` → added to `waitlistRegular_` queue
4. Booking returned with status `Waitlisted`

**Q: What happens step-by-step when cancelling a confirmed booking with people waiting?**
1. `cancelBooking("B1")` called
2. Booking B1 found, status changed to `Cancelled`
3. B1's routeId pushed to refund stack
4. Since it was Confirmed: `occupiedSeats` decremented
5. `promoteWaitlisted("R1")` called
6. VIP queue checked first → picks front person
7. `confirmSeat("R1")` now succeeds (a seat freed up)
8. That person's status → `Confirmed`

**Q: What if there are no waitlisted passengers?**
`promoteWaitlisted` checks both queues. If both are empty, it returns false. No-op.

**Q: Can the same booking be cancelled twice?**
No. `cancelBooking` checks `if (booking.status == Cancelled) return false;`

**Q: How would you add a cancel-by-refund-id feature?**
The refund stack stores booking IDs. To find a specific refund's booking, search through all bookings for the given refund ID.

**Q: What's the complexity of each operation?**
| Operation | Complexity |
|-----------|-----------|
| `bookSeat` | O(1) |
| `cancelBooking` | O(N) — linear search for booking by ID |
| `promoteWaitlisted` | O(1) |
| `allBookings` | O(N) |
