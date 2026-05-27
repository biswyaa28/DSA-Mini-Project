#ifndef STATION_DIRECTORY_HPP
#define STATION_DIRECTORY_HPP

#include "models.hpp"

#include <string>
#include <vector>

class StationDirectory {
 public:
  bool add(const Station& station);
  bool check(const std::string& stationId) const;
  const Station* find(const std::string& stationId) const;
  std::vector<Station> all() const;

  bool addStation(const Station& station);
  bool hasStation(const std::string& stationId) const;
  const Station* findStation(const std::string& stationId) const;
  std::vector<Station> allStations() const;

 private:
  std::vector<Station> stations_;
};

#endif
