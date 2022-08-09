#include "playthread.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

#define FILENAME "D:/in.pcm"
// 采样率【单位是赫兹】
#define SAMPLE_RATE 44100
// 位深度
#define SAMPLE_SIZE 16
// 声道数
#define CHANNELS 2
// 音频缓冲区的样本数量【这个是自己定的，但是这个值必须是2的幂】
#define SAMPLES 1024
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// 文件缓冲区的大小【读取一定的量，再交给 SDL 去播放】
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

PlayThread::PlayThread(QObject *parent) : QThread(parent) { connect(this, &PlayThread::finished, this, &PlayThread::deleteLater); }

PlayThread::~PlayThread() {
  disconnect();
  requestInterruption();
  quit();
  wait();
  qDebug() << this << "析构了";
}

//临时数据
int bufferLen;    //当前一次读取到的长度
char *bufferData; //当前一次读取到的数据
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
  // 采样格式（s16le，表示有符号16位小端）
  spec.format = AUDIO_S16LSB;
  // 声道数
  spec.channels = CHANNELS;
  // 音频缓冲区的样本数量（这个值必须是2的幂）
  spec.samples = SAMPLES;
  // 回调【这里使用的是拉模式，播放器自己通过设置的回调来拉数据】
  spec.callback = pull_audio_data;

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
    //尝试读取一个缓冲区大小的内容
    bufferLen = file.read(data, BUFFER_SIZE);
    //读到末尾了
    if (bufferLen <= 0) {
      break;
    }

    bufferData = data;

    //上次读好的还没有消耗完，则休眠 1 毫秒
    while (bufferLen > 0) {
      SDL_Delay(1);
    }
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

  // 文件数据还没准备好
  if (bufferLen <= 0) {
    return;
  }

  // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
  len = (len > bufferLen) ? bufferLen : len;

  // 填充数据
  SDL_MixAudio(stream, (Uint8 *)bufferData, len, SDL_MIX_MAXVOLUME);

  //修正数据
  bufferData += len; //消费了多少就移动到那个点
  bufferLen -= len;  //消费了多少就减去对应的消耗值
}
