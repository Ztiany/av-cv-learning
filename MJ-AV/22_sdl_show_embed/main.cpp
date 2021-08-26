#include "mainwindow.h"
#include <QApplication>

/** =========================================================================
使用 sdl 展示 YUV 图片【内嵌在 QT 窗口中】
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
