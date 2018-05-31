#include <QApplication>
#include "project_layout.h"

int main(int argc, char* argv[]) {
  // Make application
  QApplication project(argc,argv);
  // Make and show the widget for the window
  project_layout window("Ryan Baten Advanced Graphics Project");
  window.show();
  // Run application main loop
  return project.exec();
}
