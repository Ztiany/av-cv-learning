#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

// FFmpeg是纯C语言的
// C++是不能直接导入C语言函数的
extern "C" {
#include <libavcodec/avcodec.h>
}

/** =========================================================================
 QT 集成 FFmpeg
========================================================================= */
int main(int argc, char *argv[]) {
  //消除警告
  qputenv("QT_SCALE_FACTOR", QByteArray("1"));

  // 输出 ffmeng 信息
  // std::cout << av_version_info();，在QT中，不能使用C++标准的cout来打印信息。
  qDebug() << av_version_info();

  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
