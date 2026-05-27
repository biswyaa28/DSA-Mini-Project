#ifndef STORAGE_MANAGER_HPP
#define STORAGE_MANAGER_HPP

#include "models.hpp"

#include <vector>
#include <string>

struct StorageManager {
  bool saveBookings(const std::vector<Booking>& bookings,
                    const std::string& filepath) const;
  std::vector<Booking> loadBookings(const std::string& filepath) const;
};

#endif
