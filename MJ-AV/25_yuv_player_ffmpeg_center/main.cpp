#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

/** =========================================================================
使用 ffmpeg + qt 播放 YUV 视频
========================================================================= */

int main(int argc, char *argv[]) {

  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
