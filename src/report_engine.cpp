#include "report_engine.hpp"

std::map<std::string, double> ReportEngine::profitabilityByRoute(
    const std::vector<Booking>& bookings) {
  std::map<std::string, double> report;
  for (const auto& b : bookings) {
    report[b.routeId] += b.chargedFare;
  }
  return report;
}
