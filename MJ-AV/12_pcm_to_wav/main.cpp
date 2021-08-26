#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QThread>

extern "C" {
// 设备
#include <libavdevice/avdevice.h>
}

/** =========================================================================
使用 ffmpeg 将 PCM 转为 WAV 数据
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
