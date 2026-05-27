#include "rail_graph.hpp"
#include "station_directory.hpp"

#include <cassert>
#include <vector>

int main() {
  StationDirectory directory;
  assert(directory.add({"S0", "North"}));
  assert(directory.check("S0"));
  assert(directory.find("S0") != nullptr);
  assert(directory.all().size() == 1);

  assert(directory.addStation({"S1", "Central"}));
  assert(directory.addStation({"S2", "Harbor"}));
  assert(!directory.addStation({"S1", "Duplicate"}));
  assert(directory.hasStation("S1"));
  assert(!directory.hasStation("S3"));
  assert(directory.findStation("S2") != nullptr);
  assert(directory.findStation("S3") == nullptr);

  const std::vector<Station> stations = directory.allStations();
  assert(stations.size() == 3);

  RailGraph graph;
  assert(graph.addRoute({"R1", "S1", "S2", 10, 20.0, 100, 30}));
  assert(graph.addRoute({"R2", "S1", "S3", 12, 30.0, 50, 5}));
  assert(!graph.addRoute({"R1", "S9", "S8", 9, 9.0, 9, 1}));

  assert(graph.hasRoute("R2"));
  assert(!graph.hasRoute("R9"));
  assert(graph.findRoute("R1") != nullptr);
  assert(graph.findRoute("R9") == nullptr);

  const std::vector<Route> routes = graph.allRoutes();
  assert(routes.size() == 2);

  const std::vector<Route> outgoing = graph.outgoing("S1");
  assert(outgoing.size() == 2);
  assert(graph.outgoing("S2").empty());

  assert(graph.updateOccupiedSeats("R1", 70));
  const Route* updated = graph.findRoute("R1");
  assert(updated != nullptr);
  assert(updated->occupiedSeats == 70);

  assert(!graph.updateOccupiedSeats("R1", -1));
  assert(!graph.updateOccupiedSeats("R1", 101));
  assert(!graph.updateOccupiedSeats("R9", 1));

  return 0;
}
