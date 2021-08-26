#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 ffmpeg 进行音频重采样
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
