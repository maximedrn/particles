#pragma once

#include "Field.h"
#include <QGLViewer/qglviewer.h>
#include <memory>
#include <random>
#include <vector>

using namespace std;
using namespace qglviewer;

class ParticleSystem : public QGLViewer {
    Q_OBJECT

  public:
    enum class FieldType { Source, Sink, Vortex, Directional };
    Q_ENUM(FieldType)

    static constexpr FieldType defaultFieldType = FieldType::Source;

    explicit ParticleSystem(FieldType fieldType = defaultFieldType);
    ~ParticleSystem() override;

    static QString fieldName(FieldType fieldType);

  protected:
    void init() override;
    void draw() override;
    void animate() override;
    void keyPressEvent(QKeyEvent* event) override;
    QString helpString() const override;

  private:
    static constexpr size_t particleCount = 2000;
    static constexpr double timeStep = 0.01;

    void resetParticles();
    void resetParticle(Particle& particle);
    void createFields();
    void computeForces();
    void configureFieldManipulation();
    bool
    isAbsorbed(const Particle& particle, const Vec& previousPosition) const;

    FieldType fieldType;
    vector<Particle> particles;
    vector<unique_ptr<Field>> fields;
    mt19937 randomEngine{42};
    const Vec emitterPosition = Vec(0.0, 0.0, 0.025);
    const Vec gravity = Vec(0.0, 0.0, -9.81);
    bool gravityEnabled = true;
    bool groundCollisionsEnabled = true;
    double simulationTime = 0.0;
};
