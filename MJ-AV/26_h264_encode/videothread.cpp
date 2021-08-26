#include "videothread.h"

#include "ffmpegs.h"
#include <QDebug>

VideoThread::VideoThread(QObject *parent) : QThread(parent) {
  // 当监听到线程结束时（finished），就调用deleteLater回收内存
  connect(this, &VideoThread::finished, this, &VideoThread::deleteLater);
}

VideoThread::~VideoThread() {
  // 断开所有的连接
  disconnect();
  // 内存回收之前，正常结束线程
  requestInterruption();
  // 安全退出
  quit();
  wait();
  qDebug() << this << "析构（内存被回收）";
}

void VideoThread::run() {
  VideoEncodeSpec spec;
  spec.filename = "D:/code/av/data/data01/yuv420p_320x240.yuv";
  spec.width = 320;
  spec.height = 240;
  spec.fps = 30;
  spec.pixFmt = AV_PIX_FMT_YUV420P;
  FFmpegs::h264Encode(spec, "D:/code/av/data/data01/programming_out.h264");
}
