#include "PositionalField.h"
#include <memory>

PositionalField::PositionalField(
    const Vec& initialPosition,
    const double fieldStrength,
    const Vec& fieldColor
)
    : Field(fieldStrength), color(fieldColor) {
    this->center.setPosition(initialPosition);
}

void PositionalField::draw() const {
    static const unique_ptr<GLUquadric, decltype(&gluDeleteQuadric)> quad(
        gluNewQuadric(), &gluDeleteQuadric
    );
    if (!quad) {
        return;
    }

    glPushMatrix();
    glMultMatrixd(this->center.matrix());
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    // Keep the debug marker visible inside a dense particle cloud.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(this->color.x, this->color.y, this->color.z, 0.8f);
    gluSphere(quad.get(), PositionalField::markerRadius, 16, 12);
    glPopAttrib();
    glPopMatrix();
}
