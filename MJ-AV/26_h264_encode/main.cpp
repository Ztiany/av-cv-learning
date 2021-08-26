#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 ffmpeg 对 YUV 视频进行 H264 编码
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
