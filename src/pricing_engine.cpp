#include "pricing_engine.hpp"

namespace {
constexpr double kSurgeMultiplier = 1.3;
}

double PricingEngine::currentFare(const Route& route) const {
  if (route.isSurgePricing()) {
    return route.baseFare * kSurgeMultiplier;
  }

  return route.baseFare;
}
