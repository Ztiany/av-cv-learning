#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 使用 ffmpeg 将 AAC 音频解码为 PCM
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
