#include "DirectionalField.h"

DirectionalField::DirectionalField(
    const Vec& fieldDirection,
    const double fieldStrength,
    const double fieldAngularFrequency
)
    : Field(fieldStrength), direction(fieldDirection),
      angularFrequency(fieldAngularFrequency) {}

Vec DirectionalField::computeForce(
    const Particle& particle, const double time
) const {
    (void)particle;

    const double distance = this->direction.norm();
    if (distance < 1e-8) {
        return Vec();
    }

    const Vec normalizedDirection = this->direction / distance;
    const double modulation = std::cos(this->angularFrequency * time);

    return this->strength * modulation * normalizedDirection;
}

void DirectionalField::draw() const {
    const Vec origin(0.0, 0.0, 1.1);
    const Vec end = origin + 0.3 * this->direction;
    glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_POINT_BIT);
    glColor3f(0.3f, 1.0f, 0.3f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex3fv(origin);
    glVertex3fv(end);
    glEnd();
    glPointSize(7.0f);
    glBegin(GL_POINTS);
    glVertex3fv(end);
    glEnd();
    glPopAttrib();
}
