#include "playthread.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

/* =========================================================================
 注意事项：
        1. SDL 本身就支持解析 WAV 数据，使用 SDL 读取 WAV 数据后，播放过程与播放 PCM 数据一致。
        2. 使用 SDL 解析 WAV 时，会一次性读取所有数据。
========================================================================= */

#define FILENAME "D:/code/av/data/data01/in.wav"

PlayThread::PlayThread(QObject *parent) : QThread(parent) { connect(this, &PlayThread::finished, this, &PlayThread::deleteLater); }

PlayThread::~PlayThread() {
  disconnect();
  requestInterruption();
  quit();
  wait();
  qDebug() << this << "析构了";
}

typedef struct {
  //每次从文件读取到的长度
  int len = 0;
  //用于记录播放器每次回调拉取的数据
  int pullLen = 0;
  // Uint8 用于保证所有平台下，读取的数据都是无符号 8 位的整型。
  Uint8 *data = nullptr;
} AudioBuffer;

//回调
void pull_audio_data(void *userdata, Uint8 *stream, int len);

/**
 * SDL播放音频有 2 种模式：
 *
 * 1. Push（推）：【程序】主动推送数据给【音频设备】
 * 2. Pull（拉）：【音频设备】主动向【程序】拉取数据
 *
 * 这里采用的是拉模型。
 */
void PlayThread::run() {
  // step1：初始化子系统
  // Returns 0 on success or a negative error code on failure;
  if (SDL_Init(SDL_INIT_AUDIO)) {
    qDebug() << "SDL_Init error" << SDL_GetError();
    return;
  }

  // step2：使用 SDL 加载音频数据【PCM 会一次性读取全部数据】
  // 音频参数
  SDL_AudioSpec spec;
  // 指向音频数据的指针
  Uint8 *data = nullptr;
  // PCM 数据的长度
  Uint32 len = 0;
  if (!SDL_LoadWAV(FILENAME, &spec, &data, &len)) {
    qDebug() << "SDL_LoadWAV error" << SDL_GetError();
    // 清除所有的子系统
    SDL_Quit();
    return;
  }

  //设置样本帧的数量【2的幂】
  spec.samples = 1024;
  // 回调【这里使用的是拉模式，播放器自己通过设置的回调来拉数据】
  spec.callback = pull_audio_data;

  // 初始化用于传递给播放器的数据
  AudioBuffer audioBuffer;
  audioBuffer.data = data;
  audioBuffer.len = len;

  // 将数据设置给 spec.userdata，之后在回调中就可以拿到 audioBuffer 数据。
  spec.userdata = &audioBuffer;

  // step3：打开设备
  if (SDL_OpenAudio(&spec, nullptr)) {
    qDebug() << "SDL_OpenAudio error" << SDL_GetError();
    // 清除所有的子系统
    SDL_Quit();
    return;
  }

  // step4：开始播放【0 表示取消暂停，即开始运转】
  SDL_PauseAudio(0);

  // step5：开通填充数据
  // 每个样本的大小
  int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
  // 每个样本帧的大小
  int bytesPerSampleFrame = (sampleSize * spec.channels) >> 3;

  while (!isInterruptionRequested()) {

    //只要还没有消费完所有的数据，就继续循环。
    if (audioBuffer.len > 0) {
      continue;
    }

    //读到末尾了，等到播放器消费完读取到的数据后退出
    if (audioBuffer.len <= 0) {
      //剩余字节数 / 每个样本帧的大小 = 样本帧数
      int samples = audioBuffer.pullLen / bytesPerSampleFrame;
      //样本帧数 / 采样率【一秒处理多少个样本】 = 处理这些样本数需要的时间
      int ms = (int)(samples * 1.0F / spec.freq * 1000);
      SDL_Delay(ms);

      qDebug() << "read end: ";
      qDebug() << "          pullLen =" << audioBuffer.pullLen << "samples =" << samples << "wait for" << ms << "ms";
      break;
    }

    audioBuffer.data = (Uint8 *)data;
  }

  // step6：清理资源
  // 释放WAV文件数据
  SDL_FreeWAV(data);
  // 关闭设备
  SDL_CloseAudio();
  // 清除所有的子系统
  SDL_Quit();
}

// 等待音频设备回调（会回调多次）
void pull_audio_data(
    //数据
    void *userdata,
    // 需要往stream中填充PCM数据
    Uint8 *stream,
    // 希望填充的大小(samples * format * channels / 8)
    int len) {

  // 清空stream（静音处理）
  SDL_memset(stream, 0, len);

  //拿到数据
  AudioBuffer *buffer = (AudioBuffer *)userdata;

  // 文件数据还没准备好
  if (buffer->len <= 0) {
    return;
  }

  // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
  buffer->pullLen = (len > buffer->len) ? buffer->len : len;

  // 填充数据
  SDL_MixAudio(stream, (Uint8 *)buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);

  //修正数据
  buffer->data += buffer->pullLen; //消费了多少就移动到那个点
  buffer->len -= buffer->pullLen;  //消费了多少就减去对应的消耗值
}
