#ifndef PROJECT_LAYOUT_H
#define PROJECT_LAYOUT_H

#include <QGridLayout>
#include "project_gl.h"

class project_layout : public QWidget {
  Q_OBJECT
  private:
    int mode = 0;
    int n_modes = 3;
    projectGL *gl_widget;
    QGridLayout *layout;
  public:
    project_layout(const char* title);
};

#endif
