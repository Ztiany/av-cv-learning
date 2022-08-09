#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}

/** PCM 文件信息。*/
typedef struct {
  /** 文件名 */
  const char *filename;
  /** 采样率 */
  int sampleRate;
  /** 采样格式 */
  AVSampleFormat sampleFmt;
  /** 通道（FFmpeg中定义的单声道、立体声等标识） */
  int chLayout;
} AudioDecodeSpec;

class FFmpegs {
public:
  FFmpegs();
  static void aacDecode(const char *inFilename, AudioDecodeSpec &out);
};

#endif // FFMPEGS_H
