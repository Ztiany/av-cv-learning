#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

/* =========================================================================
 注意事项：
 1. 下面程序初始化 SwrContext 使用的 API 是 swr_alloc_set_opts，事实上，从官方示例 resampliing_audio.c
    中可以看到初始化 SwrContext 是 swr_alloc() 。【这两种方式都是可以的】
 2. 音频重采样的步骤：
    1. 初始化重采样上下文。
    2. 初始化输入缓冲区。
    3. 初始化输出缓冲区。
    4. 将输入缓冲区重采样病转换到输出缓冲区。
========================================================================= */

extern "C" {
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

#define ERROR_BUF(ret)                                                                                                                                                                                 \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

FFmpegs::FFmpegs() {}

void FFmpegs::resampleAudio(ResampleAudioSpec &in, ResampleAudioSpec &out) {
  resampleAudio(
      //输入参数
      in.filename, in.sampleRate, in.sampleFmt, in.chLayout,
      //输出参数
      out.filename, out.sampleRate, out.sampleFmt, out.chLayout);
}

void FFmpegs::resampleAudio(
    //输入参数
    const char *inFilename, int inSampleRate, AVSampleFormat inSampleFmt, int inChLayout,
    //输出参数
    const char *outFilename, int outSampleRate, AVSampleFormat outSampleFmt, int outChLayout) {

  qDebug() << "开始转换---------------------------------------------------------->";

  // step1：打开文件
  QFile inFile(inFilename);
  QFile outFile(outFilename);
  // 打开文件

  // step2：定义输入缓冲区及其相关参数
  //根据通道标识获取通道数
  int inChs = av_get_channel_layout_nb_channels(inChLayout);
  //每个采样点的字节数
  int inBytesPerSample = inChs * av_get_bytes_per_sample(inSampleFmt);
  //我们将要向缓冲区填充多少个采样点。
  int inSamples = 1024;
  //【这是一个出参】指向一个输入缓冲区的内存空间，由 ffmpeg 开辟。
  /*
   * 为什么是一个二级指针呢，因为可能存在多组音频数据，具体可以分析 av_samples_alloc_array_and_samples 方法，
   */
  uint8_t **inData = nullptr;
  //【这是一个出参】由 ffmpeg 指定缓冲区有多大（字节）
  int inLineSize = 0;
  // 读取文件数据的大小
  int len = 0;
  qDebug() << "输入缓冲区 inSampleRate = " << inSampleRate << " inSamples = " << inSamples;

  // step3：定义输出缓冲区及其相关参数
  //根据通道标识获取通道数
  int outChs = av_get_channel_layout_nb_channels(outChLayout);
  //每个采样点的字节数
  int outBytesPerSample = outChs * av_get_bytes_per_sample(outSampleFmt);
  /*
   * 对应的 inSamples 样本，经过重采样后，转换出了多少个新的样本。
   * 1. inSamples 与 outSamples 要成比例。inSamples 由开发者定，outSamples 根据 inSamples 算。
   * 2. 反过来想，不给正确的 outSamples，下面函数就不知道往 outData 里面写多少了。
   * 3. 公式为：
   *
   *   inSampleRate     inSamples
   *   ------------- = -----------
   *   outSampleRate    outSamples
   *
   *    outSamples = outSampleRate * inSamples / inSampleRate
   *
   * 4. 函数 av_rescale_rnd 用于计算 outSamples，AV_ROUND_UP 表示向上取整。
   */
  int outSamples = av_rescale_rnd(outSampleRate, inSamples, inSampleRate, AV_ROUND_UP);
  //【这是一个出参】指向一个输出缓冲区的内存空间，由 ffmpeg 开辟。
  uint8_t **outData = nullptr;
  //【这是一个出参】由 ffmpeg 指定缓冲区有多大（字节）
  int outLineSize = 0;

  qDebug() << "输入缓冲区 outSampleRate = " << outSampleRate << " outSamples = " << outSamples;

  //返回的结果
  int ret = 0;

  // step4：创建重采样上下文
  SwrContext *ctx = swr_alloc_set_opts(
      //传空，则该函数帮我们创建一个上下文，否则就是修改参数
      nullptr,
      //输出参数
      outChLayout, outSampleFmt, outSampleRate,
      //输出参数
      inChLayout, inSampleFmt, inSampleRate,
      //下面两个是日志参数，不需要
      0, nullptr);

  if (!ctx) {
    qDebug() << "swr_alloc_set_opts error";
    goto end;
  } else {
    qDebug() << "swr_alloc_set_opts success: ctx = " << ctx;
  }
  // 初始化重采样上下文
  ret = swr_init(ctx);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "swr_init error:" << errbuf;
    goto end;
  }

  // step5：创建输入缓冲区。【最后一个参数表示是否对齐，根据查看源码，发现 0 和 1 都是不对其。】
  ret = av_samples_alloc_array_and_samples(&inData, &inLineSize, inChs, inSamples, inSampleFmt, 1);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
    goto end;
  }

  // step6：创建输出缓冲区
  ret = av_samples_alloc_array_and_samples(&outData, &outLineSize, outChs, outSamples, outSampleFmt, 1);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
    goto end;
  }

  // step7：开始读数据转换
  if (!inFile.open(QFile::ReadOnly)) {
    qDebug() << "file open error:" << inFilename;
    goto end;
  }

  if (!outFile.open(QFile::WriteOnly)) {
    qDebug() << "file open error:" << outFilename;
    goto end;
  }

  while ((len = inFile.read((char *)inData[0], inLineSize)) > 0) {
    // 真实读取到的样本数量
    inSamples = len / inBytesPerSample;
    qDebug() << "read inSamples: " << inSamples;

    // outSamples 这里告诉 swr_convert 可用空间数，转换的时候存储 outSamples 个样本，如果 inSamples 还有剩余，就下次再继续。
    // 这里 ret 返回值表示转换后的样本数量
    ret = swr_convert(ctx, outData, outSamples, (const uint8_t **)inData, inSamples);
    if (ret < 0) {
      ERROR_BUF(ret);
      qDebug() << "swr_convert error:" << errbuf;
      goto end;
    }
    qDebug() << "convert outSamples: " << ret;

    //写入转换后的数据
    outFile.write((char *)outData[0], av_samples_get_buffer_size(nullptr, outChs, ret, outSampleFmt, 1)); // 等价于下面语句

    // outFile.write((char *)outData[0], ret * outBytesPerSample);
  }

  // 检查一下输出缓冲区是否还有残留的样本，注释写得很明确：
  // If more input is provided than output space, then the input will be buffered.
  while ((ret = swr_convert(ctx, outData, outSamples, nullptr, 0)) > 0) {
    qDebug() << "convert outSamples in buffer: " << ret;
    outFile.write((char *)outData[0], ret * outBytesPerSample);
  }

// step8：释放资源
end:
  // 关闭文件
  inFile.close();
  outFile.close();

  // 释放输入缓冲区
  if (inData) {
    av_freep(&inData[0]);
  }
  av_freep(&inData);

  // 释放输出缓冲区
  if (outData) {
    av_freep(&outData[0]);
  }
  av_freep(&outData);

  // 释放重采样上下文
  swr_free(&ctx);

  qDebug() << "转换结束---------------------------------------------------------->";
}
