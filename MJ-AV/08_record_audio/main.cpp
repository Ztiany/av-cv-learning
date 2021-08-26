#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <iostream>

extern "C" {
// 设备
#include <libavdevice/avdevice.h>
}

void logWays();

/** =========================================================================
 使用 ffmpeg 录制 PCM 音频
========================================================================= */
int main(int argc, char *argv[]) {

  //测试日志
  logWays();

  //打印执行线程
  qDebug() << "current Thread：" << QThread::currentThread();

  // step1：注册设备
  avdevice_register_all();

  // UI
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}

/**
 * 演示四种打印日志的方式。
 */
void logWays() {
  // C语言
  printf("log from printf\n");

  // C++
  std::cout << "log from std::cout" << std::endl;

  // FFmpeg【方式类似 C 语言】
  // 日志级别大小：TRACE < DEBUG < INFO < WARNING < ERROR < FATAL < QUIET
  av_log_set_level(AV_LOG_ERROR);
  av_log(nullptr, AV_LOG_ERROR, "log from ffmpeg AV_LOG_ERROR\n");
  av_log(nullptr, AV_LOG_WARNING, "log from ffmpeg AV_LOG_WARNING\n");
  av_log(nullptr, AV_LOG_INFO, "log from ffmpeg AV_LOG_INFO\n");

  // QT
  qDebug() << "log from qt";

  // 如果打印的日志没有打印，则刷新标准输出流
  fflush(stdout);
  fflush(stderr); // ffmpeg 使用的输出

  qDebug() << "qDebug----";
}
