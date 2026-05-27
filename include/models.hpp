#ifndef MODELS_HPP
#define MODELS_HPP

#include <algorithm>
#include <string>

struct Station {
  std::string id;
  std::string name;
};

struct Route {
  std::string id;
  std::string fromStationId;
  std::string toStationId;
  int distanceKm;
  double baseFare;
  int capacity;
  int occupiedSeats;

  int availableSeats() const {
    return std::max(0, capacity - occupiedSeats);
  }

  bool isSurgePricing() const {
    if (capacity <= 0) {
      return false;
    }

    return occupiedSeats * 100 > capacity * 80;
  }
};

enum class BookingStatus {
  Confirmed,
  Waitlisted,
  Cancelled
};

struct Booking {
  std::string id;
  std::string routeId;
  std::string passengerName;
  bool isVip;
  BookingStatus status;
  double chargedFare = 0.0;
  std::string travelDate;
};

#endif
