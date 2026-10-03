#pragma once

#include <QGLViewer/vec.h>
#include <random>

using namespace qglviewer;
using namespace std;

class Particle {
  public:
    explicit Particle(
        const Vec& initialPosition = Vec(), double particleMass = 1.0
    );

    void reset(mt19937& randomEngine, const Vec& emitterPosition);
    void draw() const;
    void computeAcceleration();
    void integrateEuler(double step);
    void handleGroundCollision();

    Vec position;
    Vec velocity;
    Vec acceleration;
    Vec force;
    const double mass;
    double age = 0.0;
    double lifetime = 1.0;
};
