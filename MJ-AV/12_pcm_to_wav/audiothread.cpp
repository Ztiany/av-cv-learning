#include "audiothread.h"
#include "ffmpegs.h"
#include <QDateTime>
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
#define FILEPATH "D:/"
#elif
#define FMT_NAME "avfoundation"
#define DEVICE_NAME ":0"
#define FILENAME "/Users/mj/Desktop/"
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
 * 结束线程
 */
void AudioThread::setStop(bool stop) { this->stop = stop; }

/**
 * 打印 AVFormatContext 内部的信息
 */
void showSpec(AVFormatContext *ctx) {
  // 获取输入流【这里只有一个音频流，所以就从 0 位置获取】
  AVStream *stream = ctx->streams[0];
  // 获取音频参数
  AVCodecParameters *params = stream->codecpar;
  // 声道数
  qDebug() << params->channels;
  // 采样率
  qDebug() << params->sample_rate;
  // 采样格式
  qDebug() << params->format;
  // 每一个样本的一个声道占用多少个字节
  qDebug() << av_get_bytes_per_sample((AVSampleFormat)params->format);
  // 编码ID（也可以看出采样格式）
  qDebug() << "codec_id：" << params->codec_id;
  qDebug() << "采样格式：" << av_get_bits_per_sample(params->codec_id);
}

void pcm2wav(AVFormatContext *ctx, QString &pcmFilePath, QString &wavFilePath) {
  // 获取输入流【这里只有一个音频流，所以就从 0 位置获取】
  AVStream *stream = ctx->streams[0];
  // 获取音频参数
  AVCodecParameters *params = stream->codecpar;

  //构建 WAV Header
  WAVHeader header;
  header.sampleRate = params->sample_rate;                         //采样率
  header.bitsPerSample = av_get_bits_per_sample(params->codec_id); //函数获取单个采样点的字节数
  header.numChannels = params->channels;                           //声道数
  // 从 FFmpeg 中 AVCodecID 枚举的定义来看，AV_CODEC_ID_PCM_F32BE 定义在 AUDIO_FORMAT_PCM 后面，所以可以通过下面公式进行判断
  // AV_CODEC_ID_PCM_F32BE 是 ffmpeg 中定义的标识，AUDIO_FORMAT_FLOAT 和 AUDIO_FORMAT_PCM 是 wav header 定义的标识
  if (params->codec_id >= AV_CODEC_ID_PCM_F32BE) {
    header.audioFormat = AUDIO_FORMAT_FLOAT;
  } else {
    header.audioFormat = AUDIO_FORMAT_PCM;
  }
  FFmpegs::pcm2wav(header, pcmFilePath.toUtf8().data(), wavFilePath.toUtf8().data());
}

// 当线程启动的时候（start），就会自动调用 run 函数
// 1. run函数中的代码是在子线程中执行的。
// 2. 耗时操作应该放在 run 函数中。
void AudioThread::run() {
  qDebug() << this << "----->录音开始执行";

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
  // 2. 这里的格式为 audio=device-name。
  const char *deviceName = DEVICE_NAME;
  int ret = avformat_open_input(&ctx, deviceName, fmt, nullptr);
  if (ret < 0) {
    char errorbuf[1024];
    av_strerror(ret, errorbuf, sizeof(errorbuf));
    qDebug() << "打开设备失败：ret = " << ret << " reason：" << errorbuf << " device name = " << deviceName;
    return;
  }

  showSpec(ctx);
  qDebug() << "得到上下文：" << ctx;

  // step4：采集数据
  QString filepath = FILEPATH;
  QString filename = filepath + QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
  QString pcmFilePath = filename + ".pcm";
  QFile pcmFile(pcmFilePath);
  qDebug() << "pcm file：" << pcmFilePath;
  ret = pcmFile.open(QFile::WriteOnly); // WriteOnly: create or truncate.

  if (ret < 0) {
    qDebug() << "打开文件失败：" << pcmFilePath;
    //释放资源
    avformat_close_input(&ctx);
    return;
  }

  qDebug() << "录音循环开始";

  // Allocate an AVPacket and set its fields to default values. The resulting struct must be freed using av_packet_free().
  AVPacket *pkt = av_packet_alloc(); //用于存放数据

  while (!isInterruptionRequested()) { //循环采集
    ret = av_read_frame(ctx, pkt);

    if (ret == 0) {
      pcmFile.write((const char *)pkt->data, pkt->size); //写入文件
    } else if (ret == AVERROR(ERANGE)) {
      // ffmpeg 返回的错误是负数，但定义的错误常量是正数，所以用 AVERROR 转一下。
      // 资源临时不可用是继续
      continue;
    } else { //否则就是错误
      char errorbuf[1024];
      av_strerror(ret, errorbuf, sizeof(errorbuf));
      qDebug() << "录音发生错误：ret = " << ret << " reason：" << errorbuf;
      break;
    }

    // Wipe the packet. Unreference the buffer referenced by the packet and reset the remaining packet fields to their default values.
    av_packet_unref(pkt);
  }
  // 释放不再需要的资源
  pcmFile.close();
  av_packet_free(&pkt);
  qDebug() << "录音循环结束";

  // step5：将录音的 pcm 转为 wav
  QString wavFilePath = filename + ".wav";
  qDebug() << "wav file：" << wavFilePath;
  pcm2wav(ctx, pcmFilePath, wavFilePath);

  // step6: 释放剩余的资源
  qDebug() << this << "转为 wav 成功";
  avformat_close_input(&ctx);

  qDebug() << this << "-----录音正常结束";
}
