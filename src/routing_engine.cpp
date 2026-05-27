#include "routing_engine.hpp"

#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

RoutingEngine::RoutingEngine(RailGraph& graph) : graph_(graph) {}

double RoutingEngine::edgeWeight(const Route& route,
                                 const std::string& mode) const {
  if (mode == "shortest") {
    return static_cast<double>(route.distanceKm);
  }

  double occupancyRatio = route.capacity > 0
                              ? static_cast<double>(route.occupiedSeats) /
                                    route.capacity
                              : 0.0;

  return route.distanceKm * (1.0 + occupancyRatio);
}

PathResult RoutingEngine::findPath(const std::string& src,
                                   const std::string& dst,
                                   const std::string& mode) const {
  PathResult result{};
  result.totalDistance = 0.0;

  if (src == dst) {
    return result;
  }

  const double INF = std::numeric_limits<double>::max();
  std::unordered_map<std::string, double> dist;
  std::unordered_map<std::string, std::string> prev;
  std::unordered_map<std::string, std::string> routeTaken;
  std::unordered_set<std::string> visited;

  dist[src] = 0.0;

  while (true) {
    std::string current;
    double minDist = INF;
    for (const auto& pair : dist) {
      if (!visited.count(pair.first) && pair.second < minDist) {
        minDist = pair.second;
        current = pair.first;
      }
    }

    if (current.empty() || current == dst) {
      break;
    }

    visited.insert(current);

    for (const Route& route : graph_.outgoing(current)) {
      double w = edgeWeight(route, mode);
      double newDist = dist[current] + w;

      if (!dist.count(route.toStationId) ||
          newDist < dist[route.toStationId]) {
        dist[route.toStationId] = newDist;
        prev[route.toStationId] = current;
        routeTaken[route.toStationId] = route.id;
      }
    }
  }

  if (!dist.count(dst)) {
    return result;
  }

  std::vector<std::string> path;
  std::string cur = dst;
  while (cur != src) {
    if (!routeTaken.count(cur)) {
      return PathResult{};
    }
    path.push_back(routeTaken[cur]);
    cur = prev[cur];
  }

  result.totalDistance = dist[dst];
  result.routeIds.assign(path.rbegin(), path.rend());
  return result;
}
