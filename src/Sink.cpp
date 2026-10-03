#include "Sink.h"

Sink::Sink(const Vec& initialPosition, const double fieldStrength)
    : PositionalField(initialPosition, fieldStrength, Vec(0.0, 1.0, 1.0)) {}

Vec Sink::computeForce(const Particle& particle, const double time) const {
    (void)time;

    const Vec radius = particle.position - this->center.position();
    const double distance = radius.norm();

    // Handle the case where the particle is at the center of the source to
    // avoid division by zero.
    if (distance < 1e-8) {
        return Vec();
    }

    const Vec direction = radius / distance;
    return -this->strength * direction / distance;
}
