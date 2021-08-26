#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 sdl 播放 wav 数据【没有用到 ffmepg】
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
