#include "sort_search.hpp"
#include "report_engine.hpp"
#include "storage_manager.hpp"
#include "models.hpp"

#include <cassert>
#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

static void test_sort_by_route_date_fare() {
  Booking b1{"B1", "R2", "Alice", false, BookingStatus::Confirmed, 150.0, "2024-03-15"};
  Booking b2{"B2", "R1", "Bob", false, BookingStatus::Confirmed, 200.0, "2024-03-10"};
  Booking b3{"B3", "R1", "Carol", false, BookingStatus::Confirmed, 100.0, "2024-03-10"};
  Booking b4{"B4", "R1", "Dave", false, BookingStatus::Confirmed, 100.0, "2024-03-01"};

  std::vector<Booking> bookings = {b1, b2, b3, b4};
  SortingAndSearch::sortByRouteDateFare(bookings);

  assert(bookings[0].id == "B4");
  assert(bookings[0].routeId == "R1");
  assert(bookings[0].travelDate == "2024-03-01");

  assert(bookings[1].id == "B3");
  assert(bookings[1].routeId == "R1");
  assert(bookings[1].travelDate == "2024-03-10");
  assert(bookings[1].chargedFare == 100.0);

  assert(bookings[2].id == "B2");
  assert(bookings[2].routeId == "R1");
  assert(bookings[2].travelDate == "2024-03-10");
  assert(bookings[2].chargedFare == 200.0);

  assert(bookings[3].id == "B1");
  assert(bookings[3].routeId == "R2");
}

static void test_binary_search_exact_match() {
  Booking b1{"B1", "R1", "A", false, BookingStatus::Confirmed, 100.0, "2024-01-01"};
  Booking b2{"B2", "R1", "B", false, BookingStatus::Confirmed, 200.0, "2024-01-01"};
  Booking b3{"B3", "R1", "C", false, BookingStatus::Confirmed, 300.0, "2024-01-01"};
  Booking b4{"B4", "R1", "D", false, BookingStatus::Confirmed, 400.0, "2024-01-01"};

  std::vector<Booking> fares = {b1, b2, b3, b4};
  auto result = SortingAndSearch::binarySearchFareRange(fares, 200.0, 300.0);

  assert(result.first == 1);
  assert(result.second == 2);
}

static void test_binary_search_no_match() {
  Booking b1{"B1", "R1", "A", false, BookingStatus::Confirmed, 100.0, "2024-01-01"};
  Booking b2{"B2", "R1", "B", false, BookingStatus::Confirmed, 200.0, "2024-01-01"};

  std::vector<Booking> fares = {b1, b2};
  auto result = SortingAndSearch::binarySearchFareRange(fares, 500.0, 600.0);

  assert(result.first == -1);
  assert(result.second == -1);
}

static void test_binary_search_full_range() {
  Booking b1{"B1", "R1", "A", false, BookingStatus::Confirmed, 100.0, "2024-01-01"};
  Booking b2{"B2", "R1", "B", false, BookingStatus::Confirmed, 200.0, "2024-01-01"};
  Booking b3{"B3", "R1", "C", false, BookingStatus::Confirmed, 300.0, "2024-01-01"};

  std::vector<Booking> fares = {b1, b2, b3};
  auto result = SortingAndSearch::binarySearchFareRange(fares, 50.0, 500.0);

  assert(result.first == 0);
  assert(result.second == 2);
}

static void test_report_profitability_by_route() {
  Booking b1{"B1", "R1", "A", false, BookingStatus::Confirmed, 100.0, "2024-01-01"};
  Booking b2{"B2", "R1", "B", false, BookingStatus::Confirmed, 200.0, "2024-01-01"};
  Booking b3{"B3", "R2", "C", false, BookingStatus::Confirmed, 300.0, "2024-01-01"};

  std::vector<Booking> bookings = {b1, b2, b3};
  auto report = ReportEngine::profitabilityByRoute(bookings);

  assert(report.size() == 2);
  assert(report["R1"] == 300.0);
  assert(report["R2"] == 300.0);
}

static void test_storage_json_roundtrip() {
  std::filesystem::create_directories("data");

  std::vector<Booking> original = {
    {"B1", "R1", "Alice", false, BookingStatus::Confirmed, 150.0, "2024-03-15"},
    {"B2", "R2", "Bob", true, BookingStatus::Waitlisted, 200.0, "2024-03-20"},
    {"B3", "R1", "Carol", false, BookingStatus::Cancelled, 100.0, "2024-03-10"}
  };

  StorageManager sm;
  sm.saveBookings(original, "data/test_bookings.json");
  std::vector<Booking> loaded = sm.loadBookings("data/test_bookings.json");

  assert(loaded.size() == 3);
  assert(loaded[0].id == "B1");
  assert(loaded[0].routeId == "R1");
  assert(loaded[0].passengerName == "Alice");
  assert(loaded[0].isVip == false);
  assert(loaded[0].status == BookingStatus::Confirmed);
  assert(loaded[0].chargedFare == 150.0);
  assert(loaded[0].travelDate == "2024-03-15");

  assert(loaded[1].id == "B2");
  assert(loaded[1].routeId == "R2");
  assert(loaded[1].passengerName == "Bob");
  assert(loaded[1].isVip == true);
  assert(loaded[1].status == BookingStatus::Waitlisted);
  assert(loaded[1].chargedFare == 200.0);
  assert(loaded[1].travelDate == "2024-03-20");

  assert(loaded[2].id == "B3");
  assert(loaded[2].routeId == "R1");
  assert(loaded[2].passengerName == "Carol");
  assert(loaded[2].isVip == false);
  assert(loaded[2].status == BookingStatus::Cancelled);
  assert(loaded[2].chargedFare == 100.0);
  assert(loaded[2].travelDate == "2024-03-10");
}

int main() {
  test_sort_by_route_date_fare();
  test_binary_search_exact_match();
  test_binary_search_no_match();
  test_binary_search_full_range();
  test_report_profitability_by_route();
  test_storage_json_roundtrip();
  return 0;
}
