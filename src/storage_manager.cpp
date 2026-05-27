#include "storage_manager.hpp"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

NLOHMANN_JSON_SERIALIZE_ENUM(BookingStatus, {
    {BookingStatus::Confirmed, "Confirmed"},
    {BookingStatus::Waitlisted, "Waitlisted"},
    {BookingStatus::Cancelled, "Cancelled"},
})

void to_json(json& j, const Booking& b) {
  j = json{
    {"id", b.id},
    {"routeId", b.routeId},
    {"passengerName", b.passengerName},
    {"isVip", b.isVip},
    {"status", b.status},
    {"chargedFare", b.chargedFare},
    {"travelDate", b.travelDate},
  };
}

void from_json(const json& j, Booking& b) {
  j.at("id").get_to(b.id);
  j.at("routeId").get_to(b.routeId);
  j.at("passengerName").get_to(b.passengerName);
  j.at("isVip").get_to(b.isVip);
  j.at("status").get_to(b.status);
  j.at("chargedFare").get_to(b.chargedFare);
  j.at("travelDate").get_to(b.travelDate);
}

bool StorageManager::saveBookings(const std::vector<Booking>& bookings,
                                   const std::string& filepath) const {
  json j = bookings;
  std::ofstream file(filepath);
  if (!file) return false;
  file << j.dump(2);
  return true;
}

std::vector<Booking> StorageManager::loadBookings(
    const std::string& filepath) const {
  std::ifstream file(filepath);
  if (!file) return {};
  json j;
  file >> j;
  return j.get<std::vector<Booking>>();
}
