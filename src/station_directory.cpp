#include "station_directory.hpp"

bool StationDirectory::add(const Station& station) {
  return addStation(station);
}

bool StationDirectory::check(const std::string& stationId) const {
  return hasStation(stationId);
}

const Station* StationDirectory::find(const std::string& stationId) const {
  return findStation(stationId);
}

std::vector<Station> StationDirectory::all() const {
  return allStations();
}

bool StationDirectory::addStation(const Station& station) {
  if (hasStation(station.id)) {
    return false;
  }

  stations_.push_back(station);
  return true;
}

bool StationDirectory::hasStation(const std::string& stationId) const {
  return findStation(stationId) != nullptr;
}

const Station* StationDirectory::findStation(const std::string& stationId) const {
  for (const Station& station : stations_) {
    if (station.id == stationId) {
      return &station;
    }
  }

  return nullptr;
}

std::vector<Station> StationDirectory::allStations() const {
  return stations_;
}
