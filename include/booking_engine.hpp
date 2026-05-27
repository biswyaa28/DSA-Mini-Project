#ifndef BOOKING_ENGINE_HPP
#define BOOKING_ENGINE_HPP

#include "models.hpp"
#include "rail_graph.hpp"

#include <deque>
#include <string>
#include <vector>

class BookingEngine {
 public:
  explicit BookingEngine(RailGraph& graph);

  Booking bookSeat(const std::string& routeId, const std::string& passengerName, bool isVip);
  bool cancelBooking(const std::string& bookingId);
  std::vector<Booking> allBookings() const;
  std::string lastRefundId() const;

 private:
  std::string nextBookingId();
  bool confirmSeat(const std::string& routeId);
  void addToWaitlist(const Booking& booking);
  bool promoteWaitlisted(const std::string& routeId);

  RailGraph& graph_;
  std::vector<Booking> bookings_;
  std::deque<Booking> waitlistVip_;
  std::deque<Booking> waitlistRegular_;
  std::vector<std::string> refundStack_;
  int bookingCounter_ = 0;
};

#endif
