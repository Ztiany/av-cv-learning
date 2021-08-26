#include "playthread.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

#define FILENAME "D:/code/av/data/data01/in.pcm"

/* =========================================================================
 注意事项：区分样本与样本帧的概念
        1. 样本就是一个采样点的大小，不考虑声道数。
        2. 样本帧考虑声道数，一个样本帧的大小 = 声道数 * 单个样本大小，如果是单声道，那么一个样本大小就等于一个样本帧的大小。
========================================================================= */

// 采样率【单位是赫兹】
#define SAMPLE_RATE 44100
// 声道数
#define CHANNELS 2

// 采样格式（s16le，表示有符号16位小端）
#define SAMPLE_FORMAT AUDIO_S16LSB
// 位深度【单位是比特位】
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 每个样本帧占用多少个字节【这里一个样本占用 4 个字节】
// #define BYTES_PER_SAMPLE_FRAME ((SAMPLE_SIZE * CHANNELS) / 8)
#define BYTES_PER_SAMPLE_FRAME ((SAMPLE_SIZE * CHANNELS) >> 3) //位移更高效

// 音频缓冲区的样本帧数量【这个是自己定的，但是这个值必须是2的幂】【样本帧的大小=样本大小 * 声道数】
// 缓冲区太大，点播放的时候，可能会有延迟，因为首先要先填充缓冲区
#define SAMPLES 2048

// 文件缓冲区的大小【单位是字节数】【读取一定的量，再交给 SDL 去播放】
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE_FRAME)

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
  //播放器每次回调拉取的数据
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
 */
void PlayThread::run() {
  // step1：初始化子系统
  // Returns 0 on success or a negative error code on failure;
  if (SDL_Init(SDL_INIT_AUDIO)) {
    qDebug() << "SDL_Init error" << SDL_GetError();
    return;
  }

  // step2：初始化播放参数
  // 音频参数
  SDL_AudioSpec spec;
  // 采样率
  spec.freq = SAMPLE_RATE;
  // 采样格式
  spec.format = SAMPLE_FORMAT;
  // 声道数
  spec.channels = CHANNELS;
  // 音频缓冲区的样本帧的数量（这个值必须是2的幂）
  spec.samples = SAMPLES;
  // 回调【这里使用的是拉模式，播放器自己通过设置的回调来拉数据】
  spec.callback = pull_audio_data;
  // 用于传递数据给播放器
  AudioBuffer audioBuffer;
  spec.userdata = &audioBuffer;

  // step3：打开设备
  if (SDL_OpenAudio(&spec, nullptr)) {
    qDebug() << "SDL_OpenAudio error" << SDL_GetError();
    // 清除所有的子系统
    SDL_Quit();
    return;
  }

  // step4：打开文件
  QFile file(FILENAME);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << FILENAME;
    // 关闭设备
    SDL_CloseAudio();
    // 清除所有的子系统
    SDL_Quit();
    return;
  }

  // step5：开始播放【0 表示取消暂停，即开始运转】
  SDL_PauseAudio(0);

  // step6：开通填充数据
  char data[BUFFER_SIZE];
  while (!isInterruptionRequested()) {

    //上次读好的还没有消耗完，则跳过此次循环
    if (audioBuffer.len > 0) {
      continue;
    }

    //尝试读取一个缓冲区大小的内容
    audioBuffer.len = file.read(data, BUFFER_SIZE);

    //读到末尾了，等到播放器消费完读取到的数据后退出
    if (audioBuffer.len <= 0) {
      //剩余字节数 / 每个样本的数量 = 样本数
      int samples = audioBuffer.pullLen / BYTES_PER_SAMPLE_FRAME;
      //样本数 / 采样率【一秒处理多少个样本】 = 处理这些样本数需要的时间
      int ms = (int)(samples * 1.0F / SAMPLE_RATE * 1000);
      SDL_Delay(ms);

      qDebug() << "read end"
               << " pullLen = " << audioBuffer.pullLen << "samples = " << samples << " wait for " << ms << " ms";
      break;
    }

    audioBuffer.data = (Uint8 *)data;
  }

  // step7：清理资源
  // 关闭文件
  file.close();
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
