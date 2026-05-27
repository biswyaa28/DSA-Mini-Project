#ifndef MST_ENGINE_HPP
#define MST_ENGINE_HPP

#include "rail_graph.hpp"

#include <string>
#include <vector>

struct MstResult {
  std::vector<std::string> routeIds;
  double totalDistance;
};

class MSTEngine {
 public:
  explicit MSTEngine(RailGraph& graph);

  MstResult buildMST(const std::string& startStation) const;

 private:
  RailGraph& graph_;
};

#endif
