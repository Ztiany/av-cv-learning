#include "videothread.h"

#include "demuxer.h"
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
  VideoDecodeSpec vOut;
  AudioDecodeSpec aOut;
  aOut.filename = "D:/code/av/data/data02/demux_programming_out.pcm"; // PCM 文件路径
  vOut.filename = "D:/code/av/data/data02/demux_programming_out.yuv"; // YUV 文件路径

  //进行封装
  Demuxer().demux("D:/code/av/data/data02/1.mp4", aOut, vOut);

  //输出解码后的视频参数
  qDebug() << "解封装后的视频";
  qDebug() << "宽" << vOut.width;
  qDebug() << "高" << vOut.height;
  qDebug() << "像素格式" << av_get_pix_fmt_name(vOut.pixFmt);
  qDebug() << "帧率" << vOut.fps;
  //输出解码后的音频参数
  qDebug() << "解封装后的音频";
  qDebug() << "采样率" << aOut.sampleRate;
  qDebug() << "通道数" << av_get_channel_layout_nb_channels(aOut.chLayout);
  qDebug() << "采样格式" << av_get_sample_fmt_name(aOut.sampleFmt);
}
