#include "booking_engine.hpp"

namespace {
bool updateBookingStatus(std::vector<Booking>& bookings, const std::string& bookingId, BookingStatus status) {
  for (Booking& booking : bookings) {
    if (booking.id == bookingId) {
      booking.status = status;
      return true;
    }
  }

  return false;
}
}

BookingEngine::BookingEngine(RailGraph& graph) : graph_(graph) {}

Booking BookingEngine::bookSeat(const std::string& routeId, const std::string& passengerName, bool isVip) {
  Booking booking{nextBookingId(), routeId, passengerName, isVip, BookingStatus::Waitlisted};

  if (confirmSeat(routeId)) {
    booking.status = BookingStatus::Confirmed;
  } else {
    addToWaitlist(booking);
  }

  bookings_.push_back(booking);
  return booking;
}

bool BookingEngine::cancelBooking(const std::string& bookingId) {
  for (Booking& booking : bookings_) {
    if (booking.id != bookingId) {
      continue;
    }

    if (booking.status == BookingStatus::Cancelled) {
      return false;
    }

    BookingStatus previousStatus = booking.status;
    booking.status = BookingStatus::Cancelled;
    refundStack_.push_back(bookingId);

    if (previousStatus == BookingStatus::Confirmed) {
      const Route* route = graph_.findRoute(booking.routeId);
      if (route) {
        graph_.updateOccupiedSeats(booking.routeId, route->occupiedSeats - 1);
      }
      promoteWaitlisted(booking.routeId);
    }

    return true;
  }

  return false;
}

std::vector<Booking> BookingEngine::allBookings() const {
  return bookings_;
}

std::string BookingEngine::lastRefundId() const {
  if (refundStack_.empty()) {
    return "";
  }

  return refundStack_.back();
}

std::string BookingEngine::nextBookingId() {
  bookingCounter_ += 1;
  return "B" + std::to_string(bookingCounter_);
}

bool BookingEngine::confirmSeat(const std::string& routeId) {
  const Route* route = graph_.findRoute(routeId);
  if (!route) {
    return false;
  }

  if (route->availableSeats() <= 0) {
    return false;
  }

  return graph_.updateOccupiedSeats(routeId, route->occupiedSeats + 1);
}

void BookingEngine::addToWaitlist(const Booking& booking) {
  if (booking.isVip) {
    waitlistVip_.push_back(booking);
  } else {
    waitlistRegular_.push_back(booking);
  }
}

bool BookingEngine::promoteWaitlisted(const std::string& routeId) {
  if (!waitlistVip_.empty()) {
    Booking booking = waitlistVip_.front();
    waitlistVip_.pop_front();
    if (confirmSeat(routeId)) {
      updateBookingStatus(bookings_, booking.id, BookingStatus::Confirmed);
      return true;
    }
    return false;
  }

  if (!waitlistRegular_.empty()) {
    Booking booking = waitlistRegular_.front();
    waitlistRegular_.pop_front();
    if (confirmSeat(routeId)) {
      updateBookingStatus(bookings_, booking.id, BookingStatus::Confirmed);
      return true;
    }
    return false;
  }

  return false;
}
