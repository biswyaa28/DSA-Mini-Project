#include "booking_engine.hpp"
#include "pricing_engine.hpp"
#include "rail_graph.hpp"

#include <cassert>

static void test_pricing_engine_surge_threshold() {
  PricingEngine engine;
  Route route{"R1", "S1", "S2", 120, 200.0, 100, 80};
  assert(engine.currentFare(route) == 200.0);

  route.occupiedSeats = 80;
  assert(engine.currentFare(route) == 200.0);

  route.occupiedSeats = 81;
  assert(engine.currentFare(route) == 260.0);
}

static void test_booking_engine_confirm_and_waitlist() {
  RailGraph graph;
  Route route{"R1", "S1", "S2", 120, 150.0, 2, 0};
  graph.addRoute(route);

  BookingEngine engine(graph);
  Booking first = engine.bookSeat("R1", "Ava", false);
  Booking second = engine.bookSeat("R1", "Ben", true);
  Booking third = engine.bookSeat("R1", "Chloe", false);

  assert(first.status == BookingStatus::Confirmed);
  assert(second.status == BookingStatus::Confirmed);
  assert(third.status == BookingStatus::Waitlisted);
}

static void test_booking_engine_vip_promotion_on_cancel() {
  RailGraph graph;
  Route route{"R1", "S1", "S2", 120, 150.0, 1, 0};
  graph.addRoute(route);

  BookingEngine engine(graph);
  Booking confirmed = engine.bookSeat("R1", "Ava", false);
  Booking vipWait = engine.bookSeat("R1", "Ben", true);
  Booking regularWait = engine.bookSeat("R1", "Chloe", false);

  assert(confirmed.status == BookingStatus::Confirmed);
  assert(vipWait.status == BookingStatus::Waitlisted);
  assert(regularWait.status == BookingStatus::Waitlisted);

  bool cancelled = engine.cancelBooking(confirmed.id);
  assert(cancelled);

  std::vector<Booking> all = engine.allBookings();
  bool vipPromoted = false;
  bool regularPromoted = false;
  for (const Booking& booking : all) {
    if (booking.id == vipWait.id) {
      vipPromoted = booking.status == BookingStatus::Confirmed;
    }
    if (booking.id == regularWait.id) {
      regularPromoted = booking.status == BookingStatus::Confirmed;
    }
  }

  assert(vipPromoted);
  assert(!regularPromoted);
}

static void test_booking_engine_regular_promotion_on_cancel_when_no_vip() {
  RailGraph graph;
  Route route{"R1", "S1", "S2", 120, 150.0, 1, 0};
  graph.addRoute(route);

  BookingEngine engine(graph);
  Booking confirmed = engine.bookSeat("R1", "Ava", false);
  Booking regularWait = engine.bookSeat("R1", "Chloe", false);

  assert(confirmed.status == BookingStatus::Confirmed);
  assert(regularWait.status == BookingStatus::Waitlisted);

  bool cancelled = engine.cancelBooking(confirmed.id);
  assert(cancelled);

  std::vector<Booking> all = engine.allBookings();
  bool regularPromoted = false;
  for (const Booking& booking : all) {
    if (booking.id == regularWait.id) {
      regularPromoted = booking.status == BookingStatus::Confirmed;
    }
  }

  assert(regularPromoted);
}

static void test_booking_engine_refund_stack_last_id() {
  RailGraph graph;
  Route route{"R1", "S1", "S2", 120, 150.0, 2, 0};
  graph.addRoute(route);

  BookingEngine engine(graph);
  Booking first = engine.bookSeat("R1", "Ava", false);
  Booking second = engine.bookSeat("R1", "Ben", false);

  bool cancelledFirst = engine.cancelBooking(first.id);
  assert(cancelledFirst);
  assert(engine.lastRefundId() == first.id);

  bool cancelledSecond = engine.cancelBooking(second.id);
  assert(cancelledSecond);
  assert(engine.lastRefundId() == second.id);
}

int main() {
  test_pricing_engine_surge_threshold();
  test_booking_engine_confirm_and_waitlist();
  test_booking_engine_vip_promotion_on_cancel();
  test_booking_engine_regular_promotion_on_cancel_when_no_vip();
  test_booking_engine_refund_stack_last_id();
  return 0;
}
