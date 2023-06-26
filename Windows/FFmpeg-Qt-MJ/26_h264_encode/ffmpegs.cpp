#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

/* =========================================================================
 注意事项：
     1：对编码后的数据进行校验，通过 FFmpeg 命令进行编码，对比两次结果，如果字节数完全一致，则说明编程正确。
     2：ffmpeg h264 编码命令：ffmpeg -s 320x240 -pix_fmt yuv420p -framerate 30 -i yuv420p_320x240.yuv -c:v libx264 ffmpeg_out.h264
     3：frame 的初始化，代码共有三种方式，其中采用方式 1 和 3 编码正常。
========================================================================= */

//方式1：该方式初始化 Frame，编码正常。
#define INIT_FRAME_WAY1

//方式2：参考官方实例中的初始化方式，但是编码后的视频存在问题，播放时有杂色。【怀疑可能该方式适用于其他格式的编码】
//#define INIT_FRAME_WAY2

//方式3：手动分配缓冲区，编码正常。
//#define INIT_FRAME_WAY3

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#define ERROR_BUF(ret)                                                                                                                                                                                 \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

FFmpegs::FFmpegs() {}

/**
 * @brief 检查编码器是否支持该像素格式。【不同的编码器仅支持特定的像素格式】
 * @return 1 表示支持，否则不支持。
 */
static int check_pix_fmt(const AVCodec *codec, enum AVPixelFormat pix_fmt) {
  // codec->sample_fmts 中保持了所有该编码器支持的格式
  const enum AVPixelFormat *p = codec->pix_fmts;

  // AV_SAMPLE_FMT_NONE 是官方定义的一个边界值，便于开发者遍历时防止越界。
  while (*p != AV_PIX_FMT_NONE) {
    qDebug() << "check_sample_fmt() checking:" << av_get_pix_fmt_name(*p);
    if (*p == pix_fmt) {
      return 1;
    }
    p++;
  }
  return 0;
}

/**
 * @brief encode 编码
 * @param ctx 上下文
 * @param frame 帧（YUV）
 * @param packet 包（H264）
 * @param outFile 输出入文件
 * @return 0 表示正常，否则表示失败。
 */
static int encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *packet, QFile &outFile) {
  // 发送数据到编码器中，该方法的说明文档如下：
  // Supply a raw video or audio frame to the encoder. Use avcodec_receive_packet() to retrieve buffered output packets.
  int ret = avcodec_send_frame(ctx, frame);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "avcodec_send_frame error" << errbuf;
    return ret;
  }

  // 不断地从编码器中取出编码后的数据【为什么是个循环，因为发送到编码器中的数据，可能无法一次性就被编码完成】
  // 从 avcodec_receive_packet 方法的返回值注释也可以看出，如果不是返回的 AVERROR_EOF 或 EAGAIN，就应该继续。
  while (true) {
    ret = avcodec_receive_packet(ctx, packet);
    // 查看 avcodec_receive_packet 的注释发现其可能的返回值：
    //    1. 0 表示成功
    //    2. AVERROR_EOF 表示 the encoder has been fully flushed, and there will be no more output packets
    //    3. EAGAIN 表示 output is not available in the current state, user must try to send input.
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      //结束当前编码循环，然后继续从文件中读取数据再进行编码。
      return 0;
    } else if (ret < 0) {
      return ret;
    }

    //编码后的数据写入文件
    outFile.write((char *)packet->data, packet->size);

    //使用 packet 中的资源
    av_packet_unref(packet);
  }
}

void FFmpegs::h264Encode(VideoEncodeSpec &in, const char *outFilename) {
  qDebug() << "开始编码---------------------------------------------------------->";
  // step1：定义需要用到的变量
  QFile inFile(in.filename);     // YUV文件
  QFile outFile(outFilename);    // H264文件
  int ret;                       //函数调用返回值
  AVCodec *codec = nullptr;      //编码器
  AVCodecContext *ctx = nullptr; //编码上下文
  AVFrame *frame = nullptr;      //存储编码前的数据（YUV）
  AVPacket *packet = nullptr;    //存储编码后的数据（H264）

  int imgSize = av_image_get_buffer_size(in.pixFmt, in.width, in.height, 1); // 一帧图片的大小
  uint8_t *buf = nullptr;                                                    //仅用于方式 3 的 frame 初始化

  // step2：获取编码器
  // codec = avcodec_find_encoder(AV_CODEC_ID_AAC);//这里找打的是默认的编码器
  codec = avcodec_find_encoder_by_name("libx264");
  if (!codec) {
    qDebug() << "libx264 encoder not found";
    return;
  }

  // step3：检测当前编码器是否支持对应的采样格式【不同的编码器对采样格式有不同的要求】
  // libfdk_aac 编码器对输入数据的要求：采样格式必须是 16 位整数。
  if (!check_pix_fmt(codec, in.pixFmt)) {
    qDebug() << "unsupported sample format" << av_get_pix_fmt_name(in.pixFmt);
    return;
  }

  // step4：根据编码器创建对应的编码上下文【函数后面的 3 说明该函数是第 3 版，查看之前的版本，发现确实是有被标为废弃】
  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    qDebug() << "avcodec_alloc_context3 error";
    return;
  }

  // step5：设置编码参数
  ctx->width = in.width;        //宽【必须】
  ctx->height = in.height;      //高【必须】
  ctx->pix_fmt = in.pixFmt;     //像素格式【必须】
  ctx->time_base = {1, in.fps}; //置帧率（1秒钟显示的帧数是in.fps）
  // 设置编码时 GOP 的数量【可选】，设置为 0 或 1 即表示只允许存在 I 帧
  // ctx->gop_size = 1;

  // step6：打开编码器【avcodec_open2 的最后一个参数 options 用于给特定编码器传递其特有的参数】
  ret = avcodec_open2(ctx, codec, nullptr);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "avcodec_open2 error" << errbuf;
    goto end;
  }

  // step7：创建 Frame，并为其创建缓冲区。
  frame = av_frame_alloc(); // struct must be freed using av_frame_free().
  if (!frame) {
    qDebug() << "av_frame_alloc error";
    goto end;
  }
  frame->width = ctx->width;
  frame->height = ctx->height;
  frame->format = ctx->pix_fmt;
  frame->pts = 0; //当前第几帧

#ifdef INIT_FRAME_WAY1
  //方法1，通过 av_image_alloc 初始化缓冲区。
  // av_image_alloc 函数说明：image buffer has to be freed by using av_freep(&pointers[0])
  ret = av_image_alloc(frame->data, frame->linesize, in.width, in.height, in.pixFmt, 1);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "av_image_alloc error" << errbuf;
    goto end;
  }
#endif

#ifdef INIT_FRAME_WAY2
  //方法2，通过 av_frame_get_buffer 初始化缓冲区。
  //利用 av_frame_get_buffer 创建缓冲区
  ret = av_frame_get_buffer(frame, 0); //最后一个参数都是传 0
  if (ret) {
    ERROR_BUF(ret);
    qDebug() << "av_frame_get_buffer error" << errbuf;
    goto end;
  }
#endif

#ifdef INIT_FRAME_WAY3
  //方法3，手动分配缓冲区。
  buf = (uint8_t *)av_malloc(imgSize);
  //让 frame 的 data 指向 buf
  av_image_fill_arrays(frame->data, frame->linesize, buf, in.pixFmt, in.width, in.height, 1);
#endif

  // step8：创建 AVPacket
  packet = av_packet_alloc();
  if (!packet) {
    qDebug() << "av_packet_alloc error";
    goto end;
  }

  // step9：打开文件
  if (!inFile.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << in.filename;
    goto end;
  }
  if (!outFile.open(QFile::WriteOnly)) {
    qDebug() << "file open error" << outFilename;
    goto end;
  }

  qDebug() << "frame's linesize = " << imgSize;

  // step10：读取数据到 frame 中，并进行编码【每次读取一帧的数据，相比音频编码一次读取多个样本帧，对于视频，只要是标准的视频数据，就不会出现读取不足的情况】
  while ((ret = inFile.read((char *)frame->data[0], imgSize)) > 0) {
    //核心：进行编码
    if (encode(ctx, frame, packet, outFile) < 0) {
      goto end;
    }
    //设置帧的序号
    frame->pts++;
  }

  // 刷新缓冲区【将缓冲区中可能剩余的数据全部消耗完】
  // 参考 avcodec_send_frame 方法注释：Frame can be NULL, in which case it is considered a flush packet.
  encode(ctx, nullptr, packet, outFile);

// step11：释放资源
end:
  inFile.close();
  outFile.close();

#ifdef INIT_FRAME_WAY1
  //对应方法 1 的释放逻辑
  if (frame) {
    // av_freep() 释放指针后还会将指针值为空指针
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
  }
#endif

#ifdef INIT_FRAME_WAY2
  if (frame) {
    av_frame_free(&frame);
  }
#endif

#ifdef INIT_FRAME_WAY3
  if (buf) {
    av_freep(&buf);
  }
  if (frame) {
    av_frame_free(&frame);
  }
#endif

  av_packet_free(&packet);
  avcodec_free_context(&ctx);
  qDebug() << "编码结束---------------------------------------------------------->";
}
