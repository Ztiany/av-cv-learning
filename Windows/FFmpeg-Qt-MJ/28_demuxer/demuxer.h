#ifndef DEMUXER_H
#define DEMUXER_H

#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

/**音频输出规格*/
typedef struct {
  /**文件路径*/
  const char *filename;
  /**采样率*/
  int sampleRate;
  /**采样格式*/
  AVSampleFormat sampleFmt;
  /**通道数*/
  int chLayout;
} AudioDecodeSpec;

/**视频输出格式*/
typedef struct {
  /**文件路径*/
  const char *filename;
  /**宽*/
  int width;
  /**高*/
  int height;
  /**像素格式*/
  AVPixelFormat pixFmt;
  /**帧率*/
  int fps;
} VideoDecodeSpec;

class Demuxer {
public:
  Demuxer();

  void demux(const char *inFileanme, AudioDecodeSpec &aOut, VideoDecodeSpec &vOut);

private:
  AVFormatContext *_fmtCtx = nullptr;    //格式上下文
  AVCodecContext *_aDecodeCtx = nullptr; //音频解码器上下文
  AVCodecContext *_vDecodeCtx = nullptr; //视频解码器上下文
  int _aStreamIndex = 0;                 //选中的音频流索引
  int _vStreamIndex = 0;                 //选中的视频流索引
  QFile _aOutFile;                       //音频输出文件
  QFile _vOutFile;                       //视频输出文件
  AudioDecodeSpec *_aOut = nullptr;      // 函数音频出参
  VideoDecodeSpec *_vOut = nullptr;      // 函数视频出参

  /**当前解析出来的 Frame【音视频公用】*/
  AVFrame *_currentFrame;

  //视频解析相关变量
  uint8_t *_imageBuf[4] = {nullptr};
  int _imageLinesizes[4] = {0};
  int _imageSize = 0;

  //音频解析相关变量
  int _sampleSize = 0;
  int _sampleFrameSize = 0;

  int initVideoInfo();
  int initAudioInfo();

  int initDecoder(AVCodecContext **decodeCtx, int *streamIndex, AVMediaType type);

  int decode(AVCodecContext *decodeCtx, AVPacket *pkt, void (Demuxer::*func)());

  void writeVideoFrame();
  void writeAudioFrame();
};

#endif // DEMUXER_H
