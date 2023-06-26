#include "videothread.h"

#include "ffmpegs.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

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
  //输出产生
  VideoDecodeSpec out;
  out.filename = "D:/code/av/data/data01/decode_h264_programming_out.yuv"; // PCM 文件路径

  //进行解码
  FFmpegs::h264Decode("D:/code/av/data/data01/ffmpeg_out.h264", out);

  //输出解码后的视频参数
  qDebug() << "宽" << out.width;
  qDebug() << "高" << out.height;
  qDebug() << "像素格式" << av_get_pix_fmt_name(out.pixFmt);
  qDebug() << "帧率" << out.fps;
}
