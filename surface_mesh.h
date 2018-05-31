#ifndef SURFACE_MESH_H
#define SURFACE_MESH_H

#include <QtOpenGL>
#include "gpu_handler.h"

class surfaceMesh {
  private:
    int mesh_mode;
    int width;
    int height;
    float spacing;
    int *obstacle;
    float *mesh;
    float *heightf;
    gpu_handler *gpu;
  public:
    surfaceMesh(int w, int h, float spacing);
    ~surfaceMesh();
    void reset();
    void procedural(float time);
    void proceduralDevice(float time);
    void heightfield();
    void heightfieldDevice();
    void addHFRipple(int x, int y);
    void heightfieldObstacle();
    void heightfieldObstacleDevice();
    void drawMesh();
    void toggleMeshMode();
};

#endif
