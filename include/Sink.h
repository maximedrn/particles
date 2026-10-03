#pragma once

#include "PositionalField.h"

class Sink : public PositionalField {
  public:
    explicit Sink(const Vec& initialPosition, double fieldStrength = 1.0);

    Vec computeForce(const Particle& particle, double time) const override;
};
