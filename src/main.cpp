#include "ParticleSystem.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QMetaEnum>
#include <cstdlib>
#include <iostream>

int main(int argc, char** const argv) {
    QApplication application(argc, argv);

    const QMetaEnum fieldTypes =
        QMetaEnum::fromType<ParticleSystem::FieldType>();
    QStringList fieldNames;
    for (int index = 0; index < fieldTypes.keyCount(); ++index) {
        fieldNames.append(QString::fromLatin1(fieldTypes.key(index)).toLower());
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("Fields and particles practice scaffold");
    parser.addHelpOption();
    const QCommandLineOption fieldOption(
        "field",
        QStringLiteral("Field: %1.").arg(fieldNames.join(", ")),
        "name",
        ParticleSystem::fieldName(ParticleSystem::defaultFieldType).toLower()
    );
    parser.addOption(fieldOption);
    parser.process(application);

    const int fieldIndex = fieldNames.indexOf(parser.value(fieldOption));
    if (fieldIndex < 0 || !parser.positionalArguments().isEmpty()) {
        std::cerr << "Usage: particles [--field "
                  << fieldNames.join('|').toStdString() << "]\n";
        return EXIT_FAILURE;
    }

    const ParticleSystem::FieldType fieldType =
        static_cast<ParticleSystem::FieldType>(fieldTypes.value(fieldIndex));
    ParticleSystem viewer(fieldType);
    viewer.setWindowTitle("Fields and Particles");
    viewer.resize(900, 700);
    viewer.show();

    return application.exec();
}
