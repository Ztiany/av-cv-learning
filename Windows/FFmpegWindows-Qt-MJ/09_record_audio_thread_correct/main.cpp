#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QThread>

extern "C" {
// 设备
#include <libavdevice/avdevice.h>
}

/** =========================================================================
 使用 ffmpeg 录制 PCM 音频
========================================================================= */
int main(int argc, char *argv[]) {

  // step1：注册设备
  avdevice_register_all();

  // UI
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
