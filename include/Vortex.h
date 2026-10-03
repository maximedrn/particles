#pragma once

#include "PositionalField.h"

class Vortex : public PositionalField {
  public:
    explicit Vortex(
        const Vec& initialPosition,
        double fieldStrength = 1.0,
        const Vec& rotationAxis = Vec(0.0, 0.0, 1.0),
        double radialFieldStrength = 5.0
    );

    Vec computeForce(const Particle& particle, double time) const override;

    Vec axis;
    double radialStrength;
};
