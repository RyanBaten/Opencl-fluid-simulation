#ifndef PROJECT_GL_H
#define PROJECT_GL_H

#include <QtOpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector>
#include "surface_mesh.h"
#include <math.h>

class projectGL : public QOpenGLWidget, protected QOpenGLFunctions{
  Q_OBJECT
  private:
    int mode;
    int shader;
    int fov;
    double theta;
    double phi;
    double depth;
    int disturb_x;
    int disturb_y;
    QTimer timer;
    surfaceMesh *surface_mesh;
    QVector<QOpenGLShaderProgram*> shader_program;
  private slots:
    void tick();
  protected:
    QElapsedTimer time;
    void drawScene();
    void initializeGL();
    void paintGL();
    void doModelViewProjection();
  public:
    projectGL();
    QSize sizeHint() const {return QSize(600,600);}
  public slots:
    void setMode(int _mode);
    void setShader(int _shader);
    void setX(int _x);
    void setY(int _y);
    void addDisturbance();
    void reset();
};

#endif
