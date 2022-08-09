#include "audiothread.h"

#include "ffmpegs.h"
#include <QDebug>

AudioThread::AudioThread(QObject *parent) : QThread(parent) {
  // 当监听到线程结束时（finished），就调用deleteLater回收内存
  connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
  // 断开所有的连接
  disconnect();
  // 内存回收之前，正常结束线程
  requestInterruption();
  // 安全退出
  quit();
  wait();
  qDebug() << this << "析构（内存被回收）";
}

void AudioThread::run() {
  //输出产生
  AudioDecodeSpec spec;
  spec.filename = "D:/out.pcm"; // PCM 文件路径

  //进行解码
  FFmpegs::aacDecode("D:/code/av/data/data01/out_api_he2.aac", spec);

  //输出解码后的音频参数

  qDebug() << "采样率：" << spec.sampleRate;
  qDebug() << "采样格式：" << av_get_sample_fmt_name(spec.sampleFmt);
  qDebug() << "声道数：" << av_get_channel_layout_nb_channels(spec.chLayout);
}
