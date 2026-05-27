#include "mst_engine.hpp"
#include "rail_graph.hpp"
#include "routing_engine.hpp"

#include <cassert>
#include <string>
#include <vector>

static RailGraph makeTestGraph() {
  RailGraph graph;
  graph.addRoute({"R1", "S1", "S2", 10, 50.0, 100, 10});
  graph.addRoute({"R2", "S1", "S3", 15, 70.0, 100, 95});
  graph.addRoute({"R3", "S2", "S3", 12, 60.0, 100, 10});
  graph.addRoute({"R4", "S2", "S4", 20, 90.0, 100, 10});
  graph.addRoute({"R5", "S3", "S4", 5, 40.0, 100, 10});
  return graph;
}

static void test_routing_shortest_path() {
  RailGraph graph = makeTestGraph();
  RoutingEngine engine(graph);

  PathResult result = engine.findPath("S1", "S4", "shortest");

  assert(result.totalDistance == 20);
  assert(result.routeIds.size() == 2);
  assert(result.routeIds[0] == "R2");
  assert(result.routeIds[1] == "R5");
}

static void test_routing_shortest_single_edge() {
  RailGraph graph = makeTestGraph();
  RoutingEngine engine(graph);

  PathResult result = engine.findPath("S1", "S3", "shortest");

  assert(result.totalDistance == 15);
  assert(result.routeIds.size() == 1);
  assert(result.routeIds[0] == "R2");
}

static void test_routing_same_source_dst() {
  RailGraph graph = makeTestGraph();
  RoutingEngine engine(graph);

  PathResult result = engine.findPath("S1", "S1", "shortest");

  assert(result.totalDistance == 0);
  assert(result.routeIds.empty());
}

static void test_routing_unreachable_dst() {
  RailGraph graph = makeTestGraph();
  RoutingEngine engine(graph);

  RailGraph emptyGraph;
  RoutingEngine emptyEngine(emptyGraph);

  PathResult result = emptyEngine.findPath("S1", "S4", "shortest");

  assert(result.totalDistance == 0);
  assert(result.routeIds.empty());
}

static void test_routing_least_crowded_differs_from_shortest() {
  RailGraph graph = makeTestGraph();
  RoutingEngine engine(graph);

  PathResult shortest = engine.findPath("S1", "S4", "shortest");
  PathResult leastCrowded = engine.findPath("S1", "S4", "least_crowded");

  assert(shortest.totalDistance < leastCrowded.totalDistance);
  assert(shortest.routeIds != leastCrowded.routeIds);
}

static void test_mst_basic() {
  RailGraph graph = makeTestGraph();
  MSTEngine engine(graph);

  MstResult result = engine.buildMST("S1");

  assert(result.totalDistance == 27);
  assert(result.routeIds.size() == 3);

  bool hasR1 = false, hasR3 = false, hasR5 = false;
  for (const std::string& id : result.routeIds) {
    if (id == "R1") hasR1 = true;
    if (id == "R3") hasR3 = true;
    if (id == "R5") hasR5 = true;
  }
  assert(hasR1);
  assert(hasR3);
  assert(hasR5);
}

static void test_mst_single_node() {
  RailGraph graph;
  MSTEngine engine(graph);

  MstResult result = engine.buildMST("S1");

  assert(result.totalDistance == 0);
  assert(result.routeIds.empty());
}

int main() {
  test_routing_shortest_path();
  test_routing_shortest_single_edge();
  test_routing_same_source_dst();
  test_routing_unreachable_dst();
  test_routing_least_crowded_differs_from_shortest();
  test_mst_basic();
  test_mst_single_node();
  return 0;
}
