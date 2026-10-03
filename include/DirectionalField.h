#pragma once

#include "Field.h"

class DirectionalField : public Field {
  public:
    explicit DirectionalField(
        const Vec& fieldDirection = Vec(1.0, 0.0, 0.0),
        double fieldStrength = 1.0,
        double fieldAngularFrequency = 1.0
    );

    Vec computeForce(const Particle& particle, double time) const override;
    void draw() const override;

    Vec direction;
    double angularFrequency;
};
