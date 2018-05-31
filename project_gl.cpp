#include "project_gl.h"
#include <stdio.h>

void projectGL::drawScene() {
  // Bind Shader
  shader_program[shader]->bind();

  // Draw mesh 
  surface_mesh->drawMesh();

  // Release Shader
  shader_program[shader]->release();
}

projectGL::projectGL() {
  // initialize variables
  mode = 0;
  shader = 1;
  fov = 55;
  theta = 45;
  phi = 15;
  depth = 4;
  disturb_x = 512;
  disturb_y = 512;
  // Create the surface
  surface_mesh = new surfaceMesh(1024,1024,0.006);
  //surface_mesh = new surfaceMesh(2048,2048,0.003);
  // Set up a timer
  timer.setInterval(5);
  connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));
  timer.start();
  // Start elapsed time
  time.start();
}

// Load everything in and set default values
void projectGL::initializeGL() {
  initializeOpenGLFunctions();

  // Shader loading
  // http://doc.qt.io/qt-5/qopenglshaderprogram.html
  // Default shader
  QOpenGLShaderProgram *program = new QOpenGLShaderProgram;
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, "default.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, "default.frag");
  shader_program.push_back(program);
  // Lines
  program = new QOpenGLShaderProgram;
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, "lines.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, "lines.frag");
  shader_program.push_back(program);
  // Colors
  program = new QOpenGLShaderProgram;
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, "color.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, "color.frag");
  shader_program.push_back(program);
  // Height greyscale
  program = new QOpenGLShaderProgram;
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, "color.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, "bw.frag");
  shader_program.push_back(program);
}

// Draw everything
void projectGL::paintGL() {
  // Qt Documentation says to call glClear as soon as possible
  // in paintGL for efficiency reasons
  // Clear the screen and Z buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Enable depth test
  glEnable(GL_DEPTH_TEST);

  // Do the projection
  doModelViewProjection();

  // Allow the sceneBuilder to construct the scene
  drawScene();

  // Disables
  glDisable(GL_DEPTH_TEST);
}

// Do projection
void projectGL::doModelViewProjection() {
  // Window dimensions
  int w = width()*devicePixelRatio();
  int h = height()*devicePixelRatio();
  float asp = w/(float)h;
  // Viewport using calculated dimensions
  glViewport(0,0,w,h);
  // Set Projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // Want perspective and not orthographic
  double z_min = depth/16;
  double z_max = 16*depth;
  double y_dim = z_min*tan(fov*M_PI/360);
  double x_dim = y_dim*asp;
  glFrustum(-x_dim,x_dim,-y_dim,y_dim,z_min,z_max);
  // Set Modelview
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated(0,0,-2*depth);
  // Rotation
  glRotated(phi,1,0,0);
  glRotated(theta,0,1,0);
}

// Perform update of mesh and screen ever tick
void projectGL::tick() {
  switch(mode) {
    case 0:
      surface_mesh->procedural(time.elapsed()*.05);
      break;
    case 1:
      surface_mesh->proceduralDevice(time.elapsed()*.05);
      break;
    case 2:
      surface_mesh->heightfield();
      break;
    case 3:
      surface_mesh->heightfieldDevice();
      break;
    case 4:
      surface_mesh->heightfieldObstacle();
      break;
    case 5:
      surface_mesh->heightfieldObstacleDevice();
      break;
  }
  update();
}

// Set the mode
void projectGL::setMode(int _mode) {
  if (mode/2 != _mode/2)
    surface_mesh->reset();
  mode = _mode;
}

// Set the shader
void projectGL::setShader(int _shader) {
  shader = _shader;
}

// For slider value changes
void projectGL::setX(int _x) {
  disturb_x = _x;
}

// For slider value changes
void projectGL::setY(int _y) {
  disturb_y = _y;
}

// Add disturbance at the slider specified values
void projectGL::addDisturbance() {
  if (mode != 0 && mode != 1) {
    surface_mesh->addHFRipple(disturb_x, disturb_y);
  }
}

// Reset the surface mesh
void projectGL::reset() {
  surface_mesh->reset();
}
