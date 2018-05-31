// Based on example files from Cu CSCI 5239

#include <QComboBox>
#include <QGridLayout>
#include <QSlider>
#include <QString>
#include "project_layout.h"

project_layout::project_layout(const char* title) {
  // Set the window title to the title specified by the user
  setWindowTitle(QString(title));
  // Create the widget for handling opengl
  gl_widget = new projectGL;

  // Select mode
  QComboBox* mode_selector = new QComboBox();
  mode_selector->addItem("Procedural Host");
  mode_selector->addItem("Procedural Device");
  mode_selector->addItem("Heightfield Host");
  mode_selector->addItem("Heightfield Device");
  mode_selector->addItem("HF Obstacle");
  mode_selector->addItem("HF Obstacle Device");

  // Select shader
  QComboBox* shader_selector = new QComboBox();
  shader_selector->addItem("Plain");
  shader_selector->addItem("Lines");
  shader_selector->addItem("Color");
  shader_selector->addItem("Black/White");
  shader_selector->setCurrentIndex(1);

  // Buttons
  QPushButton* disturb = new QPushButton("Add Disturbance");
  QPushButton* reset = new QPushButton("Reset");
  QPushButton* quit = new QPushButton("Quit");

  // Sliders
  QSlider* x = new QSlider(Qt::Horizontal);
  x->setMinimum(2);
  x->setMaximum(1022);
  x->setValue(512);
  QSlider* y = new QSlider(Qt::Horizontal);
  y->setMinimum(2);
  y->setMaximum(1022);
  y->setValue(512);

  // Set the layout
  layout = new QGridLayout;
  layout->addWidget(gl_widget,0,0,8,1);
  layout->addWidget(new QLabel("Mode"),0,1);
  layout->addWidget(mode_selector,0,2);
  layout->addWidget(new QLabel("Shader"),1,1);
  layout->addWidget(shader_selector,1,2);
  layout->addWidget(new QLabel("X"),2,1);
  layout->addWidget(x,2,2);
  layout->addWidget(new QLabel("Y"),3,1);
  layout->addWidget(y,3,2);
  layout->addWidget(disturb,4,2);
  layout->addWidget(reset,5,1);
  layout->addWidget(quit,7,2);
  // Resizing options
  layout->setColumnStretch(0,100);
  layout->setColumnMinimumWidth(0,100);
  layout->setRowStretch(3,100);
  setLayout(layout);

  // Connect signals to gl_widget
  connect(mode_selector, SIGNAL(currentIndexChanged(int)), gl_widget, SLOT(setMode(int)));
  connect(shader_selector, SIGNAL(currentIndexChanged(int)), gl_widget, SLOT(setShader(int)));
  connect(x, SIGNAL(valueChanged(int)), gl_widget, SLOT(setX(int)));
  connect(y, SIGNAL(valueChanged(int)), gl_widget, SLOT(setY(int)));
  connect(disturb, SIGNAL(pressed()), gl_widget, SLOT(addDisturbance()));
  connect(reset, SIGNAL(pressed()), gl_widget, SLOT(reset()));
  connect(quit, SIGNAL(pressed()), qApp,SLOT(quit()));
}
