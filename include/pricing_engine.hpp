#ifndef PRICING_ENGINE_HPP
#define PRICING_ENGINE_HPP

#include "models.hpp"

class PricingEngine {
 public:
  double currentFare(const Route& route) const;
};

#endif
