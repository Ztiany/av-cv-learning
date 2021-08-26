#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 ffmpeg 将 H264 视频解码为 YUV
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
