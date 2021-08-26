#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 ffmpeg 将 mp4 解封装为 YUV 和 PCM
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
