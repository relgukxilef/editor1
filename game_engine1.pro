TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += object_parallel_to_source

DEFINES += GLEW_STATIC

LIBS += -lglfw3dll -lglew32s -lopengl32

QMAKE_CXXFLAGS += -Wall -Werror

include("game_engine1/ge1.pri")

SOURCES += \
    main.cpp

HEADERS += \
	typed/interfaces.h \
    typed/primitive_types.h \
    typed/glm_types.h

DISTFILES += \
    shaders/solid.fs \
    shaders/solid.vs \
    shaders/utils.fs
