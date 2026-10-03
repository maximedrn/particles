#include "ParticleSystem.h"
#include "DirectionalField.h"
#include "Sink.h"
#include "Source.h"
#include "Vortex.h"
#include <QKeyEvent>
#include <QMetaEnum>
#include <algorithm>
#include <cmath>
#include <numbers>

ParticleSystem::ParticleSystem(const FieldType fieldType)
    : fieldType(fieldType) {}

ParticleSystem::~ParticleSystem() {
    if (this->animationIsStarted()) {
        this->stopAnimation();
    }
    // Detach the frame before the fields that own it are destroyed.
    this->setManipulatedFrame(nullptr);
}

QString ParticleSystem::fieldName(const FieldType fieldType) {
    return QString::fromLatin1(
        QMetaEnum::fromType<FieldType>().valueToKey(static_cast<int>(fieldType))
    );
}

void ParticleSystem::init() {
    glDisable(GL_LIGHTING);
    glPointSize(3.0f);
    this->setGridIsDrawn();
    this->setSceneCenter(Vec(0.0, 0.0, 0.6));
    this->setSceneRadius(2.5);
    this->camera()->setViewDirection(Vec(0.0, 1.0, -0.3));
    this->camera()->setUpVector(Vec(0.0, 0.0, 1.0));
    this->showEntireScene();

    this->setShortcut(CAMERA_MODE, 0);
    this->setShortcut(ANIMATION, 0);
    this->setShortcut(EXIT_VIEWER, Qt::Key_Escape);
    this->setKeyDescription(Qt::Key_Space, "Start or pause animation");
    this->setKeyDescription(Qt::Key_R, "Reset particles and time");
    this->setKeyDescription(Qt::Key_1, "Source field");
    this->setKeyDescription(Qt::Key_2, "Sink field");
    this->setKeyDescription(Qt::Key_3, "Vortex field");
    this->setKeyDescription(Qt::Key_4, "Directional field");
    this->setAnimationPeriod(10);

    this->createFields();
    this->resetParticles();
}

void ParticleSystem::resetParticles() {
    if (this->animationIsStarted()) {
        this->stopAnimation();
    }
    this->simulationTime = 0.0;
    this->randomEngine.seed(42);
    this->particles.clear();
    this->particles.reserve(ParticleSystem::particleCount);
    for (size_t index = 0; index < ParticleSystem::particleCount; ++index) {
        this->particles.emplace_back(this->emitterPosition);
        this->resetParticle(this->particles.back());
    }
}

void ParticleSystem::resetParticle(Particle& particle) {
    if (this->fields.empty()) {
        particle.reset(this->randomEngine, this->emitterPosition);
        return;
    }

    const auto* const field =
        dynamic_cast<const PositionalField*>(this->fields.front().get());
    if (field == nullptr) {
        particle.reset(this->randomEngine, this->emitterPosition);
        return;
    }

    if (this->fieldType == FieldType::Vortex) {
        // Initial velocity moves particles off the zero-force vortex axis.
        particle.reset(this->randomEngine, field->center.position());
        return;
    }

    // Uniform sphere sampling: z in [-1, 1], angle in [0, 2*pi].
    uniform_real_distribution<double> sample(0.0, 1.0);
    const double height = 2.0 * sample(this->randomEngine) - 1.0;
    const double angle = 2.0 * numbers::pi * sample(this->randomEngine);
    const double horizontalRadius = sqrt(1.0 - height * height);
    const Vec direction(
        horizontalRadius * cos(angle), horizontalRadius * sin(angle), height
    );

    const bool source = this->fieldType == FieldType::Source;
    const double radius = source ? PositionalField::markerRadius
                                 : 0.8 + 0.4 * sample(this->randomEngine);
    particle.reset(
        this->randomEngine, field->center.position() + radius * direction
    );
    particle.velocity = source ? 0.2 * direction : Vec();
    if (source) {
        // Recycle outflow before it leaves the displayed scene.
        particle.lifetime = 0.6 + 0.4 * sample(this->randomEngine);
    }
}

void ParticleSystem::createFields() {
    this->setManipulatedFrame(nullptr);
    this->fields.clear();
    // Show radial fields in free space so all three axes remain visible.
    const bool radialField = this->fieldType == FieldType::Source ||
        this->fieldType == FieldType::Sink;
    this->gravityEnabled = !radialField;
    this->groundCollisionsEnabled = !radialField;
    switch (this->fieldType) {
    case FieldType::Source:
        this->fields.push_back(make_unique<Source>(Vec(0.7, 0.0, 0.5)));
        break;
    case FieldType::Sink:
        this->fields.push_back(make_unique<Sink>(Vec(0.5, 0.0, 0.5)));
        break;
    case FieldType::Vortex:
        this->fields.push_back(make_unique<Vortex>(Vec(0.0, 0.0, 0.5)));
        break;
    case FieldType::Directional:
        this->fields.push_back(make_unique<DirectionalField>());
        break;
    }
    this->configureFieldManipulation();
}

void ParticleSystem::draw() {
    glBegin(GL_POINTS);
    for (const Particle& particle : this->particles) {
        particle.draw();
    }
    glEnd();

    for (const unique_ptr<Field>& field : this->fields) {
        field->draw();
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    this->drawText(
        15,
        25,
        QStringLiteral("%1 | %2 | t = %3 s")
            .arg(ParticleSystem::fieldName(this->fieldType))
            .arg(this->animationIsStarted() ? "running" : "paused")
            .arg(this->simulationTime, 0, 'f', 2)
    );
    this->drawText(
        15, 45, "Space: start/pause | R: reset | 1-4: fields | H: help"
    );
}

void ParticleSystem::computeForces() {
    for (Particle& particle : this->particles) {
        // Rebuild the force at each step; do not accumulate previous steps.
        particle.force = Vec();
        for (const unique_ptr<Field>& field : this->fields) {
            particle.force +=
                field->computeForce(particle, this->simulationTime);
        }
        if (this->gravityEnabled) {
            // Add gravity once: F_gravity = m * g.
            particle.force += particle.mass * this->gravity;
        }
    }
}

void ParticleSystem::animate() {
    this->computeForces();
    for (Particle& particle : this->particles) {
        const Vec previousPosition = particle.position;
        particle.computeAcceleration();
        particle.integrateEuler(ParticleSystem::timeStep);
        if (this->groundCollisionsEnabled) {
            particle.handleGroundCollision();
        }
        particle.age += ParticleSystem::timeStep;
        if (particle.age >= particle.lifetime ||
            this->isAbsorbed(particle, previousPosition)) {
            this->resetParticle(particle);
        }
    }
    this->simulationTime += ParticleSystem::timeStep;
}

bool ParticleSystem::isAbsorbed(
    const Particle& particle, const Vec& previousPosition
) const {
    for (const unique_ptr<Field>& field : this->fields) {
        const auto* const sink = dynamic_cast<const Sink*>(field.get());
        if (sink == nullptr) {
            continue;
        }

        // Test the whole step so a fast particle cannot cross the sink.
        const Vec segment = particle.position - previousPosition;
        const double lengthSquared = segment * segment;
        const double fraction = lengthSquared > 0.0
            ? clamp(
                  ((sink->center.position() - previousPosition) * segment) /
                      lengthSquared,
                  0.0,
                  1.0
              )
            : 0.0;
        const Vec closestPosition = previousPosition + fraction * segment;
        if ((closestPosition - sink->center.position()).norm() <=
            PositionalField::markerRadius) {
            return true;
        }
    }
    return false;
}

void ParticleSystem::configureFieldManipulation() {
    /*
     * TODO 8 (optional): select a PositionalField's center with
     * setManipulatedFrame() and bind mouse translation to QGLViewer::FRAME.
     * DirectionalField has no center to manipulate.
     */
}

void ParticleSystem::keyPressEvent(QKeyEvent* const event) {
    // AZERTY digits need Shift; keypad digits carry KeypadModifier.
    const Qt::KeyboardModifiers fieldModifiers =
        Qt::ShiftModifier | Qt::KeypadModifier;
    if ((event->modifiers() & ~fieldModifiers) != Qt::NoModifier ||
        event->isAutoRepeat()) {
        QGLViewer::keyPressEvent(event);
        return;
    }

    int fieldIndex = -1;
    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4) {
        fieldIndex = event->key() - Qt::Key_1;
    } else if (event->text().size() == 1) {
        // Use generated text when the layout reports a symbol key code.
        fieldIndex = QStringLiteral("1234").indexOf(event->text());
        if (fieldIndex < 0) {
            fieldIndex =
                QStringLiteral("&é\"'").indexOf(event->text().toLower());
        }
    }

    if (fieldIndex >= 0) {
        this->fieldType = static_cast<FieldType>(fieldIndex);
        this->createFields();
        this->resetParticles();
    } else if (event->modifiers() != Qt::NoModifier) {
        QGLViewer::keyPressEvent(event);
        return;
    } else if (event->key() == Qt::Key_Space) {
        if (this->animationIsStarted()) {
            this->stopAnimation();
        } else {
            this->startAnimation();
        }
    } else if (event->key() == Qt::Key_R) {
        this->resetParticles();
    } else {
        QGLViewer::keyPressEvent(event);
        return;
    }

    event->accept();
    this->update();
}

QString ParticleSystem::helpString() const {
    return QStringLiteral(
        "<h2>Fields and Particles</h2>"
        "<p><b>Space</b>: start or pause. <b>R</b>: reset particles and "
        "time.</p>"
        "<p><b>1</b>: source, <b>2</b>: sink, <b>3</b>: vortex, "
        "<b>4</b>: directional field.</p>"
        "<p>On AZERTY, use <b>&amp; é &quot; '</b> without Shift, "
        "or hold Shift to type <b>1-4</b>. Keypad digits also work.</p>"
        "<p>Source emits from its sphere. Sink attracts and absorbs particles "
        "emitted around it. These two scenes use free space without "
        "gravity.</p>"
        "<p>Standard QGLViewer camera controls remain available.</p>"
        "<p>The remaining field laws and optional mouse manipulation are "
        "left as exercises in the source files.</p>"
    );
}
