#ifndef ROUTING_ENGINE_HPP
#define ROUTING_ENGINE_HPP

#include "rail_graph.hpp"

#include <string>
#include <vector>

struct PathResult {
  std::vector<std::string> routeIds;
  double totalDistance;
};

class RoutingEngine {
 public:
  explicit RoutingEngine(RailGraph& graph);

  PathResult findPath(const std::string& src,
                      const std::string& dst,
                      const std::string& mode) const;

 private:
  double edgeWeight(const Route& route, const std::string& mode) const;

  RailGraph& graph_;
};

#endif
