#include "mainwindow.h"
#include <QApplication>

/** =========================================================================
演示 SDL 的窗口绘制 API
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
