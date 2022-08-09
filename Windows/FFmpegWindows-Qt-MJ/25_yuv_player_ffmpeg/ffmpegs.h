#ifndef FFMPEGS_H
#define FFMPEGS_H

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/avutil.h>
}

/** 用于表示视频帧 */
typedef struct {
  char *pixels;         //像素数据
  int width;            //宽
  int height;           //高
  AVPixelFormat format; //格式
} RawVideoFrame;

/** 用于表示视频文件 */
typedef struct {
  const char *filename; //文件路径
  int width;            //宽
  int height;           //高
  AVPixelFormat format; //路径
} RawVideoFile;

/**
 * @brief 核心逻辑：将指定帧转变为另一种像素格式，比如将 YUV 转为 RBG。
 */
class FFmpegs {
public:
  FFmpegs();

  /**
   * @brief convertRawVideo 将指定的单个图像帧转变为另一种像素格式的图像帧。
   */
  static void convertRawVideo(RawVideoFrame &in, RawVideoFrame &out);

  /**
   * @brief convertRawVideo 将指定视频文件转变为另一种像素格式的视频。
   */
  static void convertRawVideo(RawVideoFile &in, RawVideoFile &out);
};

#endif // FFMPEGS_H
