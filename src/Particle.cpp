#include "Particle.h"
#include <cmath>
#include <numbers>
#include <stdexcept>

Particle::Particle(const Vec& initialPosition, const double particleMass)
    : position(initialPosition), mass(particleMass) {
    if (this->mass <= 0.0) {
        throw invalid_argument("Particle mass must be positive");
    }
}

void Particle::reset(mt19937& randomEngine, const Vec& emitterPosition) {
    uniform_real_distribution<double> sample(0.0, 1.0);
    const double angle = 2.0 * numbers::pi * sample(randomEngine);
    const double speed = 0.4 * sample(randomEngine);

    // Emit upwards, with a random horizontal direction and lifetime.
    this->position = emitterPosition;
    this->velocity =
        Vec(speed * cos(angle), speed * sin(angle), 1.0 + sample(randomEngine));
    this->acceleration = Vec();
    this->force = Vec();
    this->age = 0.0;
    this->lifetime = 2.0 + 3.0 * sample(randomEngine);
}

void Particle::draw() const {
    // ParticleSystem batches all vertices inside one GL_POINTS block.
    const float progress = static_cast<float>(this->age / this->lifetime);
    glColor3f(progress, progress, 1.0f);
    glVertex3fv(this->position);
}

void Particle::computeAcceleration() {
    this->acceleration = this->force / this->mass;
}

void Particle::integrateEuler(const double step) {
    this->velocity += step * this->acceleration;
    this->position += step * this->velocity;
}

void Particle::handleGroundCollision() {
    // Keep the ground bounce from QGLViewer's animation example.
    if (this->position.z < 0.0) {
        this->position.z = 0.0;
        if (this->velocity.z < 0.0) {
            this->velocity.z *= -0.8;
        }
    }
}
