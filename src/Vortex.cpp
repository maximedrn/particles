#include "Vortex.h"

Vortex::Vortex(
    const Vec& initialPosition,
    const double fieldStrength,
    const Vec& rotationAxis,
    const double radialFieldStrength
)
    : PositionalField(initialPosition, fieldStrength, Vec(1.0, 0.0, 1.0)),
      axis(rotationAxis), radialStrength(radialFieldStrength) {}

Vec Vortex::computeForce(const Particle& particle, const double time) const {
    (void)time;

    const Vec radius = particle.position - this->center.position();
    const double distance = radius.norm();

    // Avoid division by zero at the vortex center.
    if (distance < 1e-8) {
        return Vec();
    }

    const double axisNorm = this->axis.norm();

    if (axisNorm < 1e-8) {
        return Vec();
    }

    const Vec normalizedAxis = this->axis / axisNorm;
    const Vec radialOffset =
        radius - (radius * normalizedAxis) * normalizedAxis;
    Vec tangent = normalizedAxis ^ radius;
    const double tangentNorm = tangent.norm();

    if (tangentNorm < 1e-8) {
        return Vec();
    }

    tangent /= tangentNorm;

    const Vec tangentialForce = this->strength * tangent / distance;
    const Vec radialForce = -this->radialStrength * radialOffset;
    return tangentialForce + radialForce;
}
