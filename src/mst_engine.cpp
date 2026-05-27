#include "mst_engine.hpp"

#include <limits>
#include <string>
#include <unordered_set>
#include <vector>

MSTEngine::MSTEngine(RailGraph& graph) : graph_(graph) {}

MstResult MSTEngine::buildMST(const std::string& startStation) const {
  MstResult result{};

  const double INF = std::numeric_limits<double>::max();
  std::unordered_set<std::string> inMst;
  inMst.insert(startStation);

  while (true) {
    double bestWeight = INF;
    std::string bestRouteId;
    std::string bestDest;

    for (const std::string& node : inMst) {
      for (const Route& route : graph_.outgoing(node)) {
        if (inMst.count(route.toStationId)) {
          continue;
        }

        double w = static_cast<double>(route.distanceKm);
        if (w < bestWeight) {
          bestWeight = w;
          bestRouteId = route.id;
          bestDest = route.toStationId;
        }
      }
    }

    if (bestRouteId.empty()) {
      break;
    }

    result.routeIds.push_back(bestRouteId);
    result.totalDistance += bestWeight;
    inMst.insert(bestDest);
  }

  return result;
}
