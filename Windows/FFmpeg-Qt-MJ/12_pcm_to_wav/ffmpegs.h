#ifndef FFMPEGS_H
#define FFMPEGS_H
#include <stdint.h>

// 16 位 PCM。
#define AUDIO_FORMAT_PCM 1
// 32 位浮点 PCM。
#define AUDIO_FORMAT_FLOAT 3

// 44 字节的 WAV Header
typedef struct {

  /* ------------RIFF chunk------------ */
  // RIFF chunk 的 ID
  uint8_t riffChunkId[4] = {'R', 'I', 'F', 'F'};
  // RIFF chunk 的 data 大小，即文件总长度减去 (riffChunkId + riffChunkDataSize) 8 字节后数据部分总大小。
  uint32_t riffChunkDataSize;
  // "WAVE"
  uint8_t format[4] = {'W', 'A', 'V', 'E'};

  /* ------------fmt chunk 数据------------ */
  // fmt chunk 的 id
  uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '};
  // fmt chunk 的 data 大小：存储 PCM 数据时，是固定的 16 字节，即从 fmtChunkDataSize 到 bitsPerSample 所占用的长度。
  uint32_t fmtChunkDataSize = 16;
  // 音频编码，1 表示 PCM，3 表示 Floating Point
  uint16_t audioFormat = AUDIO_FORMAT_PCM;
  // 声道数
  uint16_t numChannels;
  // 采样率
  uint32_t sampleRate;
  // 字节率【一秒钟处理多少个字节】 = sampleRate * blockAlign
  uint32_t byteRate;
  // 一个样本的字节数 = bitsPerSample * numChannels >> 3
  uint16_t blockAlign;
  // 位深度
  uint16_t bitsPerSample;

  /* ------------data chunk------------ */
  // data chunk的id
  uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'};
  // data chunk 的 data 大小：音频数据的总长度，即文件总长度减去文件头的长度（一般是 44 字节）
  uint32_t dataChunkDataSize;

} WAVHeader;

class FFmpegs {
public:
  FFmpegs();
  static void pcm2wav(WAVHeader &header, const char *pcmFilename, const char *wavFilename);
};

#endif // FFMPEGS_H
