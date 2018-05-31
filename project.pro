HEADERS = project_gl.h project_layout.h surface_mesh.h gpu_handler.h
SOURCES = main.cpp project_gl.cpp project_layout.cpp surface_mesh.cpp gpu_handler.cpp
QT += opengl
QMAKE_CXXFLAGS += -std=c++11 
LIBS+= -lOpenCL
