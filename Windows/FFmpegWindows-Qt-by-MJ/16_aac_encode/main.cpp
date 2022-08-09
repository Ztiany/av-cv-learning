#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 ffmpeg 对 PCM 音频进行 AAC 编码
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
