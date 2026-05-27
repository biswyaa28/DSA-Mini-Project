#include "api_server.hpp"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <string>
#include <vector>

using json = nlohmann::json;

static std::string statusStr(BookingStatus s) {
  switch (s) {
    case BookingStatus::Confirmed: return "Confirmed";
    case BookingStatus::Waitlisted: return "Waitlisted";
    case BookingStatus::Cancelled: return "Cancelled";
  }
  return "Unknown";
}

static json bookingJson(const Booking& b) {
  return {
    {"id", b.id},
    {"routeId", b.routeId},
    {"passengerName", b.passengerName},
    {"isVip", b.isVip},
    {"status", statusStr(b.status)},
    {"chargedFare", b.chargedFare},
    {"travelDate", b.travelDate}
  };
}

static json okResp(const std::string& msg, const json& data = json()) {
  json j;
  j["success"] = true;
  j["message"] = msg;
  j["data"] = data;
  return j;
}

static json errResp(const std::string& msg) {
  json j;
  j["success"] = false;
  j["message"] = msg;
  j["data"] = nullptr;
  return j;
}

static void send(httplib::Response& res, const json& j, int status = 200) {
  res.status = status;
  res.set_content(j.dump(), "application/json");
}

ApiServer::ApiServer() : booking_(graph_), routing_(graph_), mst_(graph_) {
  seedSampleData();
}

void ApiServer::seedSampleData() {
  stations_.addStation({"S1", "Central"});
  stations_.addStation({"S2", "West"});
  stations_.addStation({"S3", "North"});
  stations_.addStation({"S4", "East"});
  stations_.addStation({"S5", "South"});
  graph_.addRoute({"R1", "S1", "S2", 10, 100, 50, 0});
  graph_.addRoute({"R2", "S2", "S3", 15, 150, 50, 0});
  graph_.addRoute({"R3", "S1", "S3", 40, 200, 50, 0});
  graph_.addRoute({"R4", "S3", "S4", 20, 120, 50, 0});
  graph_.addRoute({"R5", "S2", "S4", 25, 180, 50, 0});
  graph_.addRoute({"R6", "S1", "S5", 30, 250, 50, 0});
  graph_.addRoute({"R7", "S5", "S4", 10, 90, 50, 0});
}

bool ApiServer::configure(int port) {
  port_ = port;

  svr_.set_mount_point("/", "./web");

  svr_.Post("/api/stations", [this](const httplib::Request& req, httplib::Response& res) {
    try {
      json body = json::parse(req.body);
      Station s{body.at("id").get<std::string>(), body.at("name").get<std::string>()};
      if (stations_.addStation(s)) {
        send(res, okResp("Station added"));
      } else {
        send(res, errResp("Station already exists"), 409);
      }
    } catch (...) {
      send(res, errResp("Invalid input"), 400);
    }
  });

  svr_.Get("/api/stations", [this](const httplib::Request&, httplib::Response& res) {
    json arr = json::array();
    for (const auto& s : stations_.allStations()) {
      arr.push_back({{"id", s.id}, {"name", s.name}});
    }
    send(res, okResp("OK", arr));
  });

  svr_.Post("/api/routes", [this](const httplib::Request& req, httplib::Response& res) {
    try {
      json body = json::parse(req.body);
      std::string id = body.at("id").get<std::string>();
      std::string from = body.at("fromStationId").get<std::string>();
      std::string to = body.at("toStationId").get<std::string>();
      int dist = body.at("distanceKm").get<int>();
      double fare = body.at("baseFare").get<double>();
      int cap = body.at("capacity").get<int>();

      if (!stations_.hasStation(from) || !stations_.hasStation(to)) {
        send(res, errResp("Station not found"), 404);
        return;
      }
      Route route{id, from, to, dist, fare, cap, 0};
      if (graph_.addRoute(route)) {
        send(res, okResp("Route added"));
      } else {
        send(res, errResp("Route already exists"), 409);
      }
    } catch (...) {
      send(res, errResp("Invalid input"), 400);
    }
  });

  svr_.Get("/api/routes", [this](const httplib::Request&, httplib::Response& res) {
    json arr = json::array();
    for (const auto& r : graph_.allRoutes()) {
      arr.push_back({
        {"id", r.id},
        {"fromStationId", r.fromStationId},
        {"toStationId", r.toStationId},
        {"distanceKm", r.distanceKm},
        {"baseFare", r.baseFare},
        {"capacity", r.capacity},
        {"occupiedSeats", r.occupiedSeats},
        {"availableSeats", r.availableSeats()},
        {"surge", r.isSurgePricing()}
      });
    }
    send(res, okResp("OK", arr));
  });

  svr_.Post("/api/bookings", [this](const httplib::Request& req, httplib::Response& res) {
    try {
      json body = json::parse(req.body);
      std::string routeId = body.at("routeId").get<std::string>();
      std::string passengerName = body.at("passengerName").get<std::string>();
      bool isVip = body.value("isVip", false);
      std::string travelDate = body.value("travelDate", "");

      const Route* route = graph_.findRoute(routeId);
      if (!route) {
        send(res, errResp("Route not found"), 404);
        return;
      }

      double fare = pricing_.currentFare(*route);
      Booking booking = booking_.bookSeat(routeId, passengerName, isVip);
      booking.chargedFare = fare;
      booking.travelDate = travelDate;

      json data = bookingJson(booking);
      data["chargedFare"] = fare;
      send(res, okResp("Booking created", data));
    } catch (...) {
      send(res, errResp("Invalid input"), 400);
    }
  });

  svr_.Post("/api/cancellations", [this](const httplib::Request& req, httplib::Response& res) {
    try {
      json body = json::parse(req.body);
      std::string bookingId = body.at("bookingId").get<std::string>();
      if (booking_.cancelBooking(bookingId)) {
        json data = {{"refundId", booking_.lastRefundId()}};
        send(res, okResp("Booking cancelled", data));
      } else {
        send(res, errResp("Booking not found or already cancelled"), 404);
      }
    } catch (...) {
      send(res, errResp("Invalid input"), 400);
    }
  });

  svr_.Get("/api/bookings", [this](const httplib::Request&, httplib::Response& res) {
    json arr = json::array();
    for (const auto& b : booking_.allBookings()) {
      arr.push_back(bookingJson(b));
    }
    send(res, okResp("OK", arr));
  });

  svr_.Get("/api/waitlist", [this](const httplib::Request&, httplib::Response& res) {
    json arr = json::array();
    for (const auto& b : booking_.allBookings()) {
      if (b.status != BookingStatus::Waitlisted) continue;
      arr.push_back(bookingJson(b));
    }
    send(res, okResp("OK", arr));
  });

  svr_.Get("/api/routes/search", [this](const httplib::Request& req, httplib::Response& res) {
    std::string from = req.get_param_value("from");
    std::string to = req.get_param_value("to");
    std::string mode = req.get_param_value("mode");

    if (from.empty() || to.empty()) {
      send(res, errResp("Missing from or to parameter"), 400);
      return;
    }
    if (mode != "shortest" && mode != "least_crowded") {
      send(res, errResp("Mode must be 'shortest' or 'least_crowded'"), 400);
      return;
    }
    if (!stations_.hasStation(from) || !stations_.hasStation(to)) {
      send(res, errResp("Station not found"), 404);
      return;
    }

    PathResult result = routing_.findPath(from, to, mode);
    if (result.routeIds.empty() && from != to) {
      send(res, errResp("No path found"), 404);
      return;
    }

    json data;
    data["from"] = from;
    data["to"] = to;
    data["mode"] = mode;
    data["routeIds"] = result.routeIds;
    data["totalDistance"] = result.totalDistance;
    send(res, okResp("OK", data));
  });

  svr_.Get("/api/mst", [this](const httplib::Request& req, httplib::Response& res) {
    std::string start = req.get_param_value("start");
    if (start.empty()) {
      send(res, errResp("Missing start parameter"), 400);
      return;
    }
    if (!stations_.hasStation(start)) {
      send(res, errResp("Station not found"), 404);
      return;
    }
    MstResult result = mst_.buildMST(start);
    json data;
    data["startStation"] = start;
    data["routeIds"] = result.routeIds;
    data["totalDistance"] = result.totalDistance;
    send(res, okResp("OK", data));
  });

  svr_.Get("/api/reports/profitability", [this](const httplib::Request&, httplib::Response& res) {
    auto bookings = booking_.allBookings();
    for (auto& b : bookings) {
      const Route* route = graph_.findRoute(b.routeId);
      if (route) b.chargedFare = pricing_.currentFare(*route);
    }
    auto report = ReportEngine::profitabilityByRoute(bookings);
    json data = json::object();
    for (const auto& [routeId, profit] : report) {
      data[routeId] = profit;
    }
    send(res, okResp("OK", data));
  });

  svr_.Get("/api/fares/range", [this](const httplib::Request& req, httplib::Response& res) {
    try {
      double minFare = std::stod(req.get_param_value("min"));
      double maxFare = std::stod(req.get_param_value("max"));

      auto bookings = booking_.allBookings();
      for (auto& b : bookings) {
        const Route* route = graph_.findRoute(b.routeId);
        if (route) b.chargedFare = pricing_.currentFare(*route);
      }

      std::vector<Booking> byFare = bookings;
      std::sort(byFare.begin(), byFare.end(), [](const Booking& a, const Booking& b) {
        return a.chargedFare < b.chargedFare;
      });

      auto range = SortingAndSearch::binarySearchFareRange(byFare, minFare, maxFare);
      json arr = json::array();
      if (range.first != -1) {
        for (int i = range.first; i <= range.second; i++) {
          arr.push_back(bookingJson(byFare[i]));
        }
      }
      json data;
      data["minFare"] = minFare;
      data["maxFare"] = maxFare;
      data["results"] = arr;
      data["count"] = static_cast<int>(arr.size());
      send(res, okResp("OK", data));
    } catch (...) {
      send(res, errResp("Invalid fare range"), 400);
    }
  });

  svr_.Get("/api/occupancy-map", [this](const httplib::Request&, httplib::Response& res) {
    json arr = json::array();
    for (const auto& r : graph_.allRoutes()) {
      double pct = r.capacity > 0 ? (100.0 * r.occupiedSeats / r.capacity) : 0.0;
      arr.push_back({
        {"routeId", r.id},
        {"fromStationId", r.fromStationId},
        {"toStationId", r.toStationId},
        {"capacity", r.capacity},
        {"occupiedSeats", r.occupiedSeats},
        {"availableSeats", r.availableSeats()},
        {"occupancyPercent", pct}
      });
    }
    send(res, okResp("OK", arr));
  });

  svr_.Get("/api/fare-trend", [this](const httplib::Request& req, httplib::Response& res) {
    std::string routeId = req.get_param_value("routeId");
    std::string daysStr = req.get_param_value("days");
    if (routeId.empty() || daysStr.empty()) {
      send(res, errResp("Missing routeId or days parameter"), 400);
      return;
    }

    const Route* route = graph_.findRoute(routeId);
    if (!route) {
      send(res, errResp("Route not found"), 404);
      return;
    }

    int days = std::stoi(daysStr);
    if (days <= 0 || days > 30) {
      send(res, errResp("Days must be between 1 and 30"), 400);
      return;
    }

    json arr = json::array();
    int simulated = route->occupiedSeats;
    int step = std::max(1, (route->capacity - route->occupiedSeats) / std::max(1, days));
    for (int d = 1; d <= days; d++) {
      Route r = *route;
      r.occupiedSeats = std::min(r.capacity, simulated);
      double fare = pricing_.currentFare(r);
      double pct = r.capacity > 0 ? (100.0 * r.occupiedSeats / r.capacity) : 0.0;
      arr.push_back({
        {"day", d},
        {"projectedFare", fare},
        {"projectedOccupancy", r.occupiedSeats},
        {"occupancyPercent", pct}
      });
      simulated += step;
    }
    json data;
    data["routeId"] = routeId;
    data["days"] = days;
    data["trend"] = arr;
    send(res, okResp("OK", data));
  });
  return true;
}

void ApiServer::run() {
  svr_.listen("0.0.0.0", port_);
}
