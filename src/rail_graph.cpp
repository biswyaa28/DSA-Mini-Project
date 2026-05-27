#include "rail_graph.hpp"

bool RailGraph::addRoute(const Route& route) {
  if (hasRoute(route.id)) {
    return false;
  }

  routes_.push_back(route);
  return true;
}

bool RailGraph::hasRoute(const std::string& routeId) const {
  return findRoute(routeId) != nullptr;
}

const Route* RailGraph::findRoute(const std::string& routeId) const {
  for (const Route& route : routes_) {
    if (route.id == routeId) {
      return &route;
    }
  }

  return nullptr;
}

std::vector<Route> RailGraph::allRoutes() const {
  return routes_;
}

std::vector<Route> RailGraph::outgoing(const std::string& stationId) const {
  return outgoingRoutes(stationId);
}

std::vector<Route> RailGraph::outgoingRoutes(const std::string& stationId) const {
  std::vector<Route> outgoing;

  for (const Route& route : routes_) {
    if (route.fromStationId == stationId) {
      outgoing.push_back(route);
    }
  }

  return outgoing;
}

bool RailGraph::updateOccupiedSeats(const std::string& routeId, int occupiedSeats) {
  for (Route& route : routes_) {
    if (route.id != routeId) {
      continue;
    }

    if (occupiedSeats < 0 || occupiedSeats > route.capacity) {
      return false;
    }

    route.occupiedSeats = occupiedSeats;
    return true;
  }

  return false;
}
