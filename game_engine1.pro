TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += object_parallel_to_source

DEFINES += GLEW_STATIC

LIBS += -lglfw3dll -lglew32s -lopengl32

include(game_engine1/ge1.pri)

SOURCES += \
    main.cpp \
	editor/io/obj_file.cpp \
	editor/operations/dolly_view.cpp \
	editor/operations/pan_view.cpp \
    editor/operation.cpp \
    editor/operations/drag_vertex.cpp \
    editor/operations/add_vertex.cpp \
    editor/data/mesh.cpp \
    editor/data/view.cpp \
    editor/data/context.cpp \
    editor/data/object.cpp \
    editor/operations/select_vertex.cpp \
    editor/operations/add_face.cpp \
    editor/operations/delete_vertices.cpp \
    editor/operations/rotate_view.cpp

HEADERS += \
	editor/io/obj_file.h \
	editor/operations/dolly_view.h \
    editor/operations/pan_view.h \
    editor/operation.h \
    editor/operations/drag_vertex.h \
    editor/operations/add_vertex.h \
    editor/data/mesh.h \
    editor/data/view.h \
    editor/data/context.h \
    editor/data/object.h \
    editor/operations/select_vertex.h \
    editor/operations/add_face.h \
    editor/algorithm/selection.h \
    buffer_vector.h \
    editor/operations/delete_vertices.h \
    editor/algorithm/mapping.h \
    editor/operations/rotate_view.h

DISTFILES += \
    shaders/outline.cs \
    shaders/solid.fs \
    shaders/solid.vs \
    shaders/utils.fs \
    shaders/point_handle.fs \
    shaders/point_handle.vs \
    shaders/edge_handle.fs \
	shaders/edge_handle.vs \
	shaders/obj.vs \
	shaders/obj.fs \
