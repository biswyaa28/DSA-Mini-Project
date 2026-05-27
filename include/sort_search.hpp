#ifndef SORT_SEARCH_HPP
#define SORT_SEARCH_HPP

#include "models.hpp"

#include <vector>
#include <utility>

struct SortingAndSearch {
  static void sortByRouteDateFare(std::vector<Booking>& bookings);
  static std::pair<int, int> binarySearchFareRange(
      const std::vector<Booking>& sortedFares,
      double minFare,
      double maxFare);
};

#endif
