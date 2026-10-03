#pragma once

#include "Field.h"
#include <QGLViewer/manipulatedFrame.h>

class PositionalField : public Field {
  public:
    static constexpr double markerRadius = 0.05;

    PositionalField(
        const Vec& initialPosition, double fieldStrength, const Vec& fieldColor
    );

    void draw() const override;

    // The force and drawing must read center.position() after a mouse move.
    ManipulatedFrame center;

  protected:
    const Vec color;
};
