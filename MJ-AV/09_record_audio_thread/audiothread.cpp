#include "audiothread.h"
#include <QDebug>
#include <QFile>

extern "C" {
//格式
#include <libavformat/avformat.h>
//工具类
#include <libavutil/avutil.h>
}

#ifdef Q_OS_WIN
#define FMT_NAME "dshow"
#define DEVICE_NAME                                                                                                                                                                                    \
  "audio=@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave_{E3818887-"                                                                                                                           \
  "13AE-453D-A18A-DDA52BE80DF3}"
#define FILENAME "D:/out.pcm"
#elif
#define FMT_NAME "avfoundation"
#define DEVICE_NAME ":0"
#define FILENAME "/Users/mj/Desktop/out.pcm"
#endif

AudioThread::AudioThread(QObject *parent) : QThread(parent) {
  // 当监听到线程结束时（finished），就调用deleteLater回收内存：
  // 1.如果不监听这个信号，那么所有启动的线程只能在窗口关闭时才会释放内存，从而造成内存驻留。
  // 2.多次 deleteLater 不会有问题，QT 内部已经做好处理。
  connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
  //断开所有的连接
  disconnect();

  //线程结束的第 2 种情况：
  // 1. 关闭窗口，窗口就会调用线程的析构函数，但其实线程可能还在运行。
  // 2. 因此这里要先通知线程结束自己。
  requestInterruption();
  // 3. 线程结束需要一定的时间，所以这里先等待一下。
  quit();
  wait();

  //打印日志
  qDebug() << this << "<-----析构（内存被回收）";
}

/**
 * @brief AudioThread::setStop 设置是否停止线程
 * @param stop 是否停止线程
 */
void AudioThread::setStop(bool stop) { this->stop = stop; }

// 当线程启动的时候（start），就会自动调用run函数
// 1. run函数中的代码是在子线程中执行的。
// 2. 耗时操作应该放在run函数中
void AudioThread::run() {
  qDebug() << this << "----->开始执行";

  // step2：获取输入格式对象：dshow 这种设备，在代码中就用 AVInputFormat 表示。
  //【在 windows 平台用 ffmpeg -devices 列出设备驱动，一般就是 dshow】
  AVInputFormat *fmt = av_find_input_format(FMT_NAME);
  if (!fmt) {
    qDebug() << "获取输入格式对象失败" << FMT_NAME;
    return;
  }
  qDebug() << "得到 AVInputFormat：" << fmt;

  // step3：打开设备得到上下文【可以使用上下文操作设备】
  AVFormatContext *ctx = nullptr; //这里一定要初始化，否则 ctx 不会被赋值
  // ffmpeg -f dshow -list_devices true -i dumy 得到的名称
  // 这里的格式为 audio=device-name，具体参考
  // https://stackoverflow.com/questions/16618686/directshow-capture-source-and-ffmpeg
  const char *deviceName = DEVICE_NAME;
  int ret = avformat_open_input(&ctx, deviceName, fmt, nullptr);
  if (ret < 0) {
    char errorbuf[1024];
    av_strerror(ret, errorbuf, sizeof(errorbuf));
    qDebug() << "打开设备失败：ret = " << ret << " reason：" << errorbuf << " device name = " << deviceName;
    return;
  }

  qDebug() << "得到上下文：" << ctx;

  // step4：采集数据
  const char *filename = FILENAME;
  QFile file(filename);
  ret = file.open(QFile::WriteOnly); // WriteOnly: create or truncate.
  if (ret < 0) {
    qDebug() << "打开文件失败：" << filename;
    //释放资源
    avformat_close_input(&ctx);
    return;
  }
  qDebug() << "开始录音";
  AVPacket pkt; //用于存放数据

  //方案1：使用自定义的结束标识
  // while (!stop && av_read_frame(ctx, &pkt) == 0) { //循环采集
  // file.write((const char *)pkt.data, pkt.size);  //写入文件
  //}

  //方案2：使用自带的结束标识
  while (!isInterruptionRequested() && av_read_frame(ctx, &pkt) == 0) { //循环采集
    file.write((const char *)pkt.data, pkt.size);                       //写入文件
  }

  // step5: 释放资源
  qDebug() << "录音结束";
  file.close();
  avformat_close_input(&ctx);

  qDebug() << this << "-----正常结束";
}
