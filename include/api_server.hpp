#ifndef API_SERVER_HPP
#define API_SERVER_HPP

#include "station_directory.hpp"
#include "rail_graph.hpp"
#include "pricing_engine.hpp"
#include "booking_engine.hpp"
#include "routing_engine.hpp"
#include "mst_engine.hpp"
#include "sort_search.hpp"
#include "report_engine.hpp"
#include "storage_manager.hpp"

#include <httplib.h>
#include <string>

class ApiServer {
public:
  ApiServer();
  bool configure(int port);
  void run();

private:
  void seedSampleData();
  httplib::Server svr_;
  StationDirectory stations_;
  RailGraph graph_;
  PricingEngine pricing_;
  BookingEngine booking_;
  RoutingEngine routing_;
  MSTEngine mst_;
  StorageManager storage_;
  int port_ = 8080;
};

#endif
