#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

/* =========================================================================
解码过程：
         文件中的 H264 数据 --> 解析器 --> 解码器 --> YUV 音频数据。

注意：
   （1）decode 方法中最后的 write 步骤仅适用于 YUV420P 的数据。
   （2）在循环读取文件时，下面代码会造成最终解码出的数据少一帧的情况，根据
        https://patchwork.ffmpeg.org/project/ffmpeg/patch/tencent_609A2E9F73AB634ED670392DD89A63400008@qq.com/ 中的方法
        将循环读取文件的代码改为 do-while 模式。

//    while ((inLen = inFile.read(inDataArray, IN_DATA_SIZE)) > 0)
//        while (inLen > 0) {
//            // 经过解析器解析
//            ret = av_parser_parse2(parserCtx, ctx,
//                                   &pkt->data, &pkt->size,
//                                   (uint8_t *) inData, inLen,
//                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

//            if (ret < 0) {
//                ERROR_BUF(ret);
//                qDebug() << "av_parser_parse2 error" << errbuf;
//                goto end;
//            }

//            // 跳过已经解析过的数据
//            inData += ret;
//            // 减去已经解析过的数据大小
//            inLen -= ret;

//            // 解码
//            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
//                goto end;
//            }
//        }
//    }
    ========================================================================= */

#define WRITE_YUV_WAT3 //【这种方式写 YUV420 就没有问题】

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

// 输入缓冲区的大小【4096是参考的官方示例】
#define IN_DATA_SIZE 4096

#define ERROR_BUF(ret)                                                                                                                                                                                 \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

static int frameIdx = 0;

FFmpegs::FFmpegs() {}

/**
 * @brief decode 解码的过程
 * @param ctx 上下文
 * @param pkt 解码前的数据（h264）
 * @param frame 解码后的数据（yuv）
 * @param outFile 输出文件
 * @return 0 表示成功
 */
static int decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, QFile &outFile) {
  //发送数据到解码器：Supply raw packet data as input to a decoder.
  int ret = avcodec_send_packet(ctx, pkt);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "avcodec_send_packet error" << errbuf;
    return ret;
  }

  //不断地从解码器中获取解码数据，直到所有的数据都解码完毕才返回。
  while (true) {
    /*
     * avcodec_receive_frame 方法的作用：Return decoded output data from a decoder.
     *
     *   返回值说明：
     *
     *     AVERROR(EAGAIN)：output is not available in this state - user must try to send new input. 表示需要更多新的数据。
     *     AVERROR_EOF：the encoder has been flushed, and no new frames can be sent to it. 表示没有更多数据了
     */
    ret = avcodec_receive_frame(ctx, frame);

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) { //数据已经被处理完毕
      return 0;
    } else if (ret < 0) { //发生错误
      ERROR_BUF(ret);
      qDebug() << "avcodec_receive_frame error" << errbuf;
      return ret;
    }

    qDebug() << "解码出第" << ++frameIdx << "帧";
    qDebug() << "frame->linesize[0]" << frame->linesize[0];
    qDebug() << "frame->linesize[1]" << frame->linesize[1];
    qDebug() << "frame->linesize[2]" << frame->linesize[2];
    qDebug() << "y 平面的大小" << (frame->linesize[0] * ctx->height) << "u 平面的大小" << (frame->data[2] - frame->data[1]);
    //如果解码出来的存放 frame 中的数据是连续的。
    // 则 frame->data[1] - frame->data[0] = y平面的大小；frame->data[2] - frame->data[1] = u平面的大小
    qDebug() << "frame->data[0]" << frame->data[0] << "frame->data[1]" << frame->data[1] << "frame->data[2]" << frame->data[2];
    qDebug() << "实际解析：frame->data[1] - frame->data[0]" << (frame->data[1] - frame->data[0]);
    qDebug() << "实际解析：frame->data[2] - frame->data[1]" << (frame->data[2] - frame->data[1]);

#ifdef WRITE_YUV_WAT1
    // 解码后的数据写入到文件【错误，因为解码出来的存放 frame 中的数据不是连续的。】
    // int imgSize = av_image_get_buffer_size(ctx->pix_fmt, ctx->width, ctx->height, 1);
    // outFile.write((char *) frame->data[0], imgSize);
#endif

#ifdef WRITE_YUV_WAT2
    // 将解码后的数据写入文件【下面代码仅适用于 YUV420】【有问题】
    //如果解码后的数据是按如下结构排列的，则下面代码可正常允许【某些 ffmpeg 版本是这样的】
    /*

                   YYYYYYYY
                   YYYYYYYY
                   YYYYYYYY
                   YYYYYYYY000000
                   UUUU
                   UUUU000
                   VVVV
                   VVVV000

     */
    //    写入Y平面
    //    outFile.write((char *)frame->data[0], frame->linesize[0] * ctx->height);
    //    // 写入U平面
    //    outFile.write((char *)frame->data[1], frame->linesize[1] * ctx->height >> 1);
    //    // 写入V平面
    //    outFile.write((char *)frame->data[2], frame->linesize[2] * ctx->height >> 1);
#endif

#ifdef WRITE_YUV_WAT3
    // 将解码后的数据写入文件【下面代码仅适用于 YUV420】【正常】
    // 参考：https://www.programmersought.com/article/13864791210/
    /*
         解码后 avFrame 中的数据可能这样的，每一行长度为 linesize[x]，但是 linesize[x] 的长度比视频真实宽高要大。

                   YYYYYYYY0000
                   YYYYYYYY0000
                   YYYYYYYY0000
                   YYYYYYYY0000
                   UUUU00
                   UUUU00
                   VVVV00
                   VVVV00

    */
    AVFrame *avFrame = frame;
    int width = avFrame->width;
    int height = avFrame->height;
    for (int i = 0; i < height; i++)
      outFile.write((char *)avFrame->data[0] + i * avFrame->linesize[0], width);
    for (int j = 0; j < height / 2; j++)
      outFile.write((char *)avFrame->data[1] + j * avFrame->linesize[1], width / 2);
    for (int k = 0; k < height / 2; k++)
      outFile.write((char *)avFrame->data[2] + k * avFrame->linesize[2], width / 2);
  }
#endif
}

void FFmpegs::h264Decode(const char *inFilename, VideoDecodeSpec &out) {
  qDebug() << "开始解码---------------------------------------------------------->";

  // step1：定义需要用到的变量
  int ret; //函数调用返回值
  // 用来存放读取的输入文件数据（h264），加上 AV_INPUT_BUFFER_PADDING_SIZE 是为了防止某些优化过的 reader 一次性读取过多导致越界。
  char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
  memset(inDataArray + IN_DATA_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
  char *inData = inDataArray; //用于记住解码到的位置

  int inLen;     //每次从输入文件中读取到的长度（h264）【也就是输入缓冲区中，剩下的等待进行解码的有效数据长度】
  int inEnd = 0; //一个标记，表示是否已经读到了输入文件的尾部【0 表示 false】

  QFile inFile(inFilename);    //输入文件（h264）
  QFile outFile(out.filename); //输出文件（yuv）

  AVCodec *codec = nullptr;                  //解码器
  AVCodecContext *ctx = nullptr;             //解码器上下文
  AVCodecParserContext *parserCtx = nullptr; //解析器上下文

  AVPacket *pkt = nullptr;  //解码前的数据（h264）
  AVFrame *frame = nullptr; //解码器后的数据（yuv）

  // step2：初始化解码器、上下文、Frame、Packet
  //方法1
  // codec = avcodec_find_decoder_by_name("h264");
  //方法2
  codec = avcodec_find_decoder(AV_CODEC_ID_H264);

  if (!codec) {
    qDebug() << "decoder not found";
    return;
  }

  //初始化解析器
  parserCtx = av_parser_init(codec->id);
  if (!parserCtx) {
    qDebug() << "av_parser_init error";
    return;
  }

  //初始化解析上下文
  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    qDebug() << "avcodec_alloc_context3 error";
    goto end;
  }

  pkt = av_packet_alloc();
  if (!pkt) {
    qDebug() << "av_packet_alloc error";
    goto end;
  }

  frame = av_frame_alloc();
  if (!frame) {
    qDebug() << "av_frame_alloc error";
    goto end;
  }

  // step3：打开解码器
  ret = avcodec_open2(ctx, codec, nullptr);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "avcodec_open2 error" << errbuf;
    goto end;
  }

  // step4：打开文件
  if (!inFile.open(QFile::ReadOnly)) {
    qDebug() << "file open error:" << inFilename;
    goto end;
  }

  if (!outFile.open(QFile::WriteOnly)) {
    qDebug() << "file open error:" << out.filename;
    goto end;
  }

  // step5：读取文件数据，并进行解析。
  do {
    inLen = inFile.read(inDataArray, IN_DATA_SIZE); //读取指定长度的数据
    inEnd = !inLen;                                 //是否到文件末尾
    inData = inDataArray;

    //情况1：只要输入缓冲区中还有数据就继续进行解码
    //情况2：缓冲区中没有数据了，但是读到了文件结尾，也要调用 av_parser_parse2 把解析器中缓冲的数据刷新出来【否则会少帧】
    while (inLen > 0 || inEnd) {
      //先给到解析器
      ret = av_parser_parse2(
          //上下文
          parserCtx, ctx,
          //存放解析后的数据
          &pkt->data, &pkt->size,
          //待解析的数据
          (uint8_t *)inData, inLen,
          //其他参数
          AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

      if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_parser_parse2 error" << errbuf;
        goto end;
      }

      inData += ret; //跳过解析过的数据
      inLen -= ret;  //减去已经解析的长度
      qDebug() << "inEnd" << inEnd << "pkt->size" << pkt->size << "ret" << ret;

      /*
       *对比 17_aac_decode，为什么这里不用设置读取阈值？因为 parser 会控制送给 avPackage 的数据，其内部会做缓冲，待读取到一定量时，才会一次性送给 avPackage。
       * 其实，解码 17_aac_decode 中 AAC 音频的时候不去设置读取阈值也可以正常解码的。【设置读取阈值的方式是参数官方实例的，怀疑这种方式是针对 Mpeg2 音频的优化】。
       */

      //开始解码
      if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
        goto end;
      }

      //如果到了文件尾部
      if (inEnd) {
        break;
      }
    }

  } while (!inEnd);

  //刷新缓冲区域（packet 传 null 即表示要刷新缓冲区）
  decode(ctx, nullptr, frame, outFile);

  // step6：赋值输出参数
  out.width = ctx->width;
  out.height = ctx->height;
  out.pixFmt = ctx->pix_fmt;
  // 用 framerate.num 获取帧率，而不是 time_base.den
  qDebug() << "ctx->framerate.den" << ctx->framerate.den << "ctx->framerate.num" << ctx->framerate.num;
  out.fps = ctx->framerate.num;

  // step7：关闭资源
end:
  inFile.close();
  outFile.close();
  av_packet_free(&pkt);
  av_frame_free(&frame);
  av_parser_close(parserCtx);
  avcodec_free_context(&ctx);
  qDebug() << "解码结束---------------------------------------------------------->";
}
