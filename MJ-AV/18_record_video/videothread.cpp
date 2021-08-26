#include "videothread.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>

extern "C" {
//格式
#include <libavformat/avformat.h>
//工具类
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#ifdef Q_OS_WIN
#define FMT_NAME "dshow"
#define DEVICE_NAME "video=Integrated Camera"
#define FILENAME "D:/"
#elif
#define FMT_NAME "avfoundation"
#define DEVICE_NAME "0"
#define FILENAME "/Users/mj/Desktop/"
#endif

//输出 FFMPEG 错误
#define ERROR_BUF(ret)                                                                                                                                                                                 \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

VideoThread::VideoThread(QObject *parent) : QThread(parent) {
  // 当监听到线程结束时（finished），就调用deleteLater回收内存：
  // 1.如果不监听这个信号，那么所有启动的线程只能在窗口关闭时才会释放内存，从而造成内存驻留。
  // 2.多次 deleteLater 不会有问题，QT 内部已经做好处理。
  connect(this, &VideoThread::finished, this, &VideoThread::deleteLater);
}

VideoThread::~VideoThread() {
  //断开所有的连接
  disconnect();

  //线程结束的第 2 种情况：关闭窗口，窗口就会调用线程的析构函数，但其实线程可能还在运行。
  // 1. 因此这里要先通知线程结束自己。
  requestInterruption();
  // 2. 线程结束需要一定的时间，所以这里先等待一下。
  quit();
  wait();

  //打印日志
  qDebug() << this << "<-----析构（内存被回收）";
}

/**
 * 打印 AVFormatContext 内部的信息
 */
void showSpec(AVFormatContext *ctx) {
  // 获取输入流【这里只有一个音频流，所以就从 0 位置获取】
  AVStream *stream = ctx->streams[0];
  // 获取视频参数
  AVCodecParameters *params = stream->codecpar;
  // 获取视频格式
  AVPixelFormat format = (AVPixelFormat)params->format;
  qDebug() << "视频宽度：" << params->width;
  qDebug() << "视频高度：" << params->height;
  qDebug() << "视频格式：" << av_pix_fmt_desc_get(format)->name;
}

/**
 * @brief getImageSize 获取一帧的大小
 * @param ctx 上下文
 * @return 一帧的大小（字节）
 */
int getImageSize(AVFormatContext *ctx) {
  // 获取输入流【这里只有一个音频流，所以就从 0 位置获取】
  AVStream *stream = ctx->streams[0];
  // 获取视频参数
  AVCodecParameters *params = stream->codecpar;
  // 视频格式
  AVPixelFormat format = (AVPixelFormat)params->format;
  int imageSize = 0;

  //方式 1:
  // imageSize = av_image_get_buffer_size(format, params->width, params->height, 1);

  //方式2：
  const AVPixFmtDescriptor *fd = av_pix_fmt_desc_get(format);
  int pixelSize = av_get_bits_per_pixel(fd);
  imageSize = (pixelSize * params->width * params->height) >> 3;

  qDebug() << "一帧的大小：" << imageSize;

  return imageSize;
}

// 当线程启动的时候（start），就会自动调用 run 函数
// 1. run函数中的代码是在子线程中执行的。
// 2. 耗时操作应该放在 run 函数中。
void VideoThread::run() {
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
  // 1. device-name 是 ffmpeg -f dshow -list_devices true -i dumy 得到的名称。
  // 2. 这里的格式为 video=device-name。
  const char *deviceName = DEVICE_NAME;

  //传递视频参数，告诉 FFmpeg，我们要录制的视频格式。
  AVDictionary *options = nullptr;
  av_dict_set(&options, "video_size", "640x360", 0);
  av_dict_set(&options, "pixel_format", "yuyv422", 0);
  av_dict_set(&options, "framerate", "30", 0);

  int ret = avformat_open_input(&ctx, deviceName, fmt, &options);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "打开设备失败";
    return;
  }

  qDebug() << "得到 AVFormatContext：" << ctx;

  // step4：采集数据
  QString filename = FILENAME;
  filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
  filename += ".yuv";
  QFile file(filename);
  ret = file.open(QFile::WriteOnly); // WriteOnly: create or truncate.

  if (ret < 0) {
    qDebug() << "打开文件失败：" << filename;
    //释放资源
    avformat_close_input(&ctx);
    return;
  }

  qDebug() << "开始录像";
  showSpec(ctx);
  // Allocate an AVPacket and set its fields to default values. The resulting struct must be freed using av_packet_free().
  AVPacket *pkt = av_packet_alloc();

  int imageSize = getImageSize(ctx);

  while (!isInterruptionRequested()) { //循环采集
    ret = av_read_frame(ctx, pkt);

    if (ret == 0) {
      //这里写入长度不适用 pkt->size 是因为，某些平台会读取冗余的信息，导致录制的视频不正常。
      file.write((const char *)pkt->data, imageSize); //写入文件
    } else if (ret == AVERROR(ERANGE)) {
      // ffmpeg 返回的错误是负数，但定义的错误常量是正数，所以用内置的 AVERROR() 宏函数转一下。
      // 资源临时不可用是继续
      continue;
    } else { //否则就是错误
      ERROR_BUF(ret);
      qDebug() << "录像发生错误";
      break;
    }

    // Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values.
    av_packet_unref(pkt);
  }

  // step5: 释放资源
  qDebug() << "录像结束";
  file.close();
  av_packet_free(&pkt);
  avformat_close_input(&ctx);

  qDebug() << this << "-----正常结束";
}
