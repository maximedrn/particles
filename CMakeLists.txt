cmake_minimum_required(VERSION 3.20)
project(particles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FIELD "" CACHE STRING "Field used by run (empty uses the application default)")
set(RUN_ARGUMENTS)
if(NOT FIELD STREQUAL "")
    list(APPEND RUN_ARGUMENTS --field "${FIELD}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.cmake")

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_QT5 QUIET Qt5Widgets)
endif()

find_package(Qt5 5.9 REQUIRED COMPONENTS Widgets OpenGL Xml
    HINTS "${PC_QT5_PREFIX}/lib/cmake/Qt5")
find_package(OpenGL REQUIRED)
include(QGLViewer)

add_executable(particles
    src/main.cpp
    src/ParticleSystem.cpp
    include/ParticleSystem.h
    src/Particle.cpp
    include/Particle.h
    src/Field.cpp
    include/Field.h
    src/PositionalField.cpp
    include/PositionalField.h
    src/Source.cpp
    include/Source.h
    src/Sink.cpp
    include/Sink.h
    src/Vortex.cpp
    include/Vortex.h
    src/DirectionalField.cpp
    include/DirectionalField.h
)
target_include_directories(particles PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(particles PRIVATE
    QT_NO_KEYWORDS
    $<$<PLATFORM_ID:Windows>:WIN32>
)
target_link_libraries(particles PRIVATE QGLViewer::QGLViewer)

if(MSVC)
    target_compile_options(particles PRIVATE /W4)
else()
    target_compile_options(particles PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(particles PROPERTIES
    AUTOMOC ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    get_target_property(PARTICLES_SOURCES particles SOURCES)
    add_custom_target(format
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" --style=file -i ${PARTICLES_SOURCES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Formatting C++ sources"
        VERBATIM
    )
    add_custom_target(format-check
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" --style=file --dry-run --Werror ${PARTICLES_SOURCES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Checking C++ formatting"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found; format and format-check targets are unavailable")
endif()

add_custom_target(run
    COMMAND "$<TARGET_FILE:particles>" ${RUN_ARGUMENTS}
    DEPENDS particles
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
    VERBATIM
)
