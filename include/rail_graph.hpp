#ifndef RAIL_GRAPH_HPP
#define RAIL_GRAPH_HPP

#include "models.hpp"

#include <string>
#include <vector>

class RailGraph {
 public:
  bool addRoute(const Route& route);
  bool hasRoute(const std::string& routeId) const;
  const Route* findRoute(const std::string& routeId) const;
  std::vector<Route> allRoutes() const;
  std::vector<Route> outgoing(const std::string& stationId) const;
  std::vector<Route> outgoingRoutes(const std::string& stationId) const;
  bool updateOccupiedSeats(const std::string& routeId, int occupiedSeats);

 private:
  std::vector<Route> routes_;
};

#endif
