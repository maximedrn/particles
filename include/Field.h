#pragma once

#include "Particle.h"

class Field {
  public:
    explicit Field(double fieldStrength = 1.0);
    virtual ~Field() = default;

    virtual Vec computeForce(const Particle& particle, double time) const = 0;
    virtual void draw() const = 0;

    double strength;
};
