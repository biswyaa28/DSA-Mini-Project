#include "models.hpp"

#include <cassert>

int main() {
  Station station{"S1", "Central"};
  assert(station.id == "S1");
  assert(station.name == "Central");

  Route route{"R1", "S1", "S2", 120, 250.0, 100, 80};
  assert(route.availableSeats() == 20);
  assert(!route.isSurgePricing());

  route.occupiedSeats = 81;
  assert(route.isSurgePricing());

  Booking booking{"B1", "R1", "Alice", true, BookingStatus::Confirmed};
  assert(booking.isVip);
  assert(booking.status == BookingStatus::Confirmed);

  return 0;
}
