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
  AudioEncodeSpec spec;
  // PCM 文件路径
  spec.filename = "D:/code/av/data/data01/44100_s16le_2.pcm";
  // 采样格式
  spec.sampleFmt = AV_SAMPLE_FMT_S16;
  // 双声道立体声
  spec.chLayout = AV_CH_LAYOUT_STEREO;
  // 采样率
  spec.sampleRate = 44100;
  FFmpegs::aacEncode(spec, "D:/out.aac");
}
