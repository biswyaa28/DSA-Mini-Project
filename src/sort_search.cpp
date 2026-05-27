#include "sort_search.hpp"

#include <algorithm>

void SortingAndSearch::sortByRouteDateFare(std::vector<Booking>& bookings) {
  std::sort(bookings.begin(), bookings.end(),
      [](const Booking& a, const Booking& b) {
        if (a.routeId != b.routeId) {
          return a.routeId < b.routeId;
        }
        if (a.travelDate != b.travelDate) {
          return a.travelDate < b.travelDate;
        }
        return a.chargedFare < b.chargedFare;
      });
}

std::pair<int, int> SortingAndSearch::binarySearchFareRange(
    const std::vector<Booking>& sortedFares,
    double minFare,
    double maxFare) {
  int lo = 0;
  int hi = static_cast<int>(sortedFares.size()) - 1;
  int start = -1;

  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    if (sortedFares[mid].chargedFare >= minFare) {
      start = mid;
      hi = mid - 1;
    } else {
      lo = mid + 1;
    }
  }

  if (start == -1) {
    return {-1, -1};
  }

  lo = start;
  hi = static_cast<int>(sortedFares.size()) - 1;
  int end = -1;

  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    if (sortedFares[mid].chargedFare <= maxFare) {
      end = mid;
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }

  if (end == -1) {
    return {-1, -1};
  }

  return {start, end};
}
