#ifndef REPORT_ENGINE_HPP
#define REPORT_ENGINE_HPP

#include "models.hpp"

#include <vector>
#include <map>
#include <string>

struct ReportEngine {
  static std::map<std::string, double> profitabilityByRoute(
      const std::vector<Booking>& bookings);
};

#endif
