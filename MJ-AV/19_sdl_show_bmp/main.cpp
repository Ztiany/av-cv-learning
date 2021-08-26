#include "mainwindow.h"
#include <QApplication>

/** =========================================================================
使用 sdl 展示 BMP 图片
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
