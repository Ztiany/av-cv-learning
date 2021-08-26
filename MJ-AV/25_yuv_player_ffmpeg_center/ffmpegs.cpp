#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

//展示 FFmpeg 错误
#define ERR_BUF                                                                                                                                                                                        \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

//处理错误并结束程序。
#define END(func)                                                                                                                                                                                      \
  if (ret < 0) {                                                                                                                                                                                       \
    ERR_BUF;                                                                                                                                                                                           \
    qDebug() << #func << "error" << errbuf;                                                                                                                                                            \
    goto end;                                                                                                                                                                                          \
  }

FFmpegs::FFmpegs() {}

/*
-----------------------------------
下面注释用于帮助理解各种像素格式的数据排列。
-----------------------------------

-----------------------------------
640*480，yuv420p。数据排列如下：

---- 640个Y -----
YY............YY |
YY............YY |
YY............YY |
YY............YY
................ 480行
YY............YY
YY............YY |
YY............YY |
YY............YY |
YY............YY |

---- 320个U -----
UU............UU |
UU............UU |
UU............UU |
UU............UU
................ 240行
UU............UU
UU............UU |
UU............UU |
UU............UU |
UU............UU |

---- 320个V -----
VV............VV |
VV............VV |
VV............VV |
VV............VV
................ 240行
VV............VV
VV............VV |
VV............VV |
VV............VV |
VV............VV |


-----------------------------------
6 * 4，yuv420p。简化后的数据排序：

YYYYYY
YYYYYY
YYYYYY
YYYYYY
UUU
UUU
VVV
VVV


-----------------------------------
600*600，rgb24。数据排列如下：

------- 600个RGB ------
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB
RGB RGB .... RGB RGB 600行
RGB RGB .... RGB RGB
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
*/

void FFmpegs::convertRawVideo(RawVideoFrame &in, RawVideoFrame &out) {

  SwsContext *ctx = nullptr; //上下文

  uint8_t *inData[4];  //输入缓冲区【数组长度 4 是参数指定的】【每个元素指向一个平面，平面数取决于像素格式】
  uint8_t *outData[4]; //输出缓冲区【数组长度 4 是参数指定的】【每个元素指向一个平面，平面数取决于像素格式】

  int inStrides[4];  //输入缓冲区每个平面的 linesize，比如 YUV 的大小分别为 320 160 160。【linesize 即一行有多少个 Y/U/V】
  int outStrides[4]; //输出缓冲区每个平面的 linesize。如果是 RGB，那么只有一个平面。

  int inFrameSize, outFrameSize; //每张图片的大小

  int ret = 0;

  //初始化上下文
  ctx = sws_getContext(
      //输入信息
      in.width, in.height, in.format,
      //输出信息
      out.width, out.height, out.format,
      // specify which algorithm and options to use for rescaling
      SWS_BILINEAR, //双线性的, 双直线的
      //滤镜
      nullptr, nullptr,
      // param extra parameters to tune the used scaler
      nullptr);

  if (!ctx) {
    qDebug() << "sws_getContext error";
    goto end;
  }

  //初始化输入缓冲区
  ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
  END(av_image_alloc)

  //初始化输出缓冲区
  ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
  END(av_image_alloc)

  //计算每张图片的大小
  inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
  outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

  qDebug() << "inSize" << in.width << "*" << in.height;
  qDebug() << "inStrides" << inStrides[0] << inStrides[1] << inStrides[2] << inStrides[3];
  qDebug() << "outSize" << out.width << "*" << out.height;
  qDebug() << "outStrides" << outStrides[0] << outStrides[1] << outStrides[2] << outStrides[3];

  //拷贝输入数据
  memcpy(inData[0], in.pixels, inFrameSize);

  //进行转换
  sws_scale(
      //上下文
      ctx,
      //入参：数据，平面数
      inData, inStrides,
      //从第几个元素开始处理数据，一般都是 0
      0,
      //原图像高度
      in.height,
      //出参：接收数据的缓冲区，平面数
      outData, outStrides);

  //复制转换后的数据到输出参数【out.pixels 的内存由调用者使用】
  out.pixels = (char *)malloc(outFrameSize);
  memcpy(out.pixels, outData[0], outFrameSize);

end:
  av_freep(&inData[0]);
  av_freep(&outData[0]);
  sws_freeContext(ctx);
}

void FFmpegs::convertRawVideo(RawVideoFile &in, RawVideoFile &out) {
  // 上下文
  SwsContext *ctx = nullptr;

  // 输入、输出缓冲区（指向每一个平面的数据）
  uint8_t *inData[4], *outData[4];

  // 每一个平面的大小
  int inStrides[4], outStrides[4];

  // 每一帧图片的大小
  int inFrameSize, outFrameSize;

  // 返回结果
  int ret = 0;

  // 进行到了那一帧
  int frameIdx = 0;
  // 文件
  QFile inFile(in.filename), outFile(out.filename);

  // 创建上下文
  ctx = sws_getContext(in.width, in.height, in.format, out.width, out.height, out.format, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!ctx) {
    qDebug() << "sws_getContext error";
    goto end;
  }

  // 输入缓冲区
  ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
  END(av_image_alloc);

  // 输出缓冲区
  ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
  END(av_image_alloc);

  // 打开文件
  if (!inFile.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << in.filename;
    goto end;
  }

  if (!outFile.open(QFile::WriteOnly)) {
    qDebug() << "file open error" << out.filename;
    goto end;
  }

  // 计算每一帧图片的大小
  inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
  outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

  // 进行每一帧的转换
  while (inFile.read((char *)inData[0], inFrameSize) == inFrameSize) {
    // 转换
    sws_scale(ctx, inData, inStrides, 0, in.height, outData, outStrides);
    // 写到输出文件去
    outFile.write((char *)outData[0], outFrameSize);
    qDebug() << "转换完第" << frameIdx++ << "帧";
  }

end:
  inFile.close();
  outFile.close();
  av_freep(&inData[0]);
  av_freep(&outData[0]);
  sws_freeContext(ctx);
}
