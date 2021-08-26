#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

/* =========================================================================
解码过程：
         文件中的 AAC 数据 --> 解析器 --> 解码器 --> PCM 音频数据。
========================================================================= */

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

// 输入缓冲区的大小【一次性从文件读取多少】
#define IN_DATA_SIZE 20480
// 已经解析的数据缓冲区中，剩余多少时，则再次读取输入文件数据。
#define REFILL_THRESHOLD 4096

#define ERROR_BUF(ret)                                                                                                                                                                                 \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

FFmpegs::FFmpegs() {}

/**
 * @brief decode 解码的过程
 * @param ctx 上下文
 * @param pkt 解码前的数据（aac）
 * @param frame 解码后的数据（pcm）
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

  qDebug() << "start decode";
  int times = 0;

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

    times++;
    qDebug() << "decode times" << times;

    // 解码后的数据写入到文件
    // frame->lineSize：For audio, size in bytes of each plane.
    outFile.write((char *)frame->data[0], frame->linesize[0]);
  }
}

void FFmpegs::aacDecode(const char *inFilename, AudioDecodeSpec &out) {
  qDebug() << "开始解码---------------------------------------------------------->";

  // step1：定义需要用到的变量
  int ret; //函数调用返回值
  // 用来存放读取的输入文件数据（aac）,加上 AV_INPUT_BUFFER_PADDING_SIZE 是为了防止某些优化过的 reader 一次性读取过多导致越界。
  char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
  char *inData = inDataArray; //用于记住解码到的位置

  int inLen;     //每次从输入文件中读取到的长度（aac）
  int inEnd = 0; //一个标记，表示是否已经读到了输入文件的尾部【0 表示 false】

  QFile inFile(inFilename);    //输入文件（aac）
  QFile outFile(out.filename); //输出文件（pcm）

  AVCodec *codec = nullptr;                  //解码器
  AVCodecContext *ctx = nullptr;             //解码器上下文
  AVCodecParserContext *parserCtx = nullptr; //解析器上下文

  AVPacket *pkt = nullptr;  //解码前的数据（aac）
  AVFrame *frame = nullptr; //解码器后的数据（pcm）

  // step2：初始化解码器、上下文、Frame、Packet
  //   解码器：
  //      aac 默认输出 fltp
  //      fdk_aac 默认输出 s16le
  codec = avcodec_find_decoder_by_name("libfdk_aac");
  if (!codec) {
    qDebug() << "decoder not found";
    return;
  }

  parserCtx = av_parser_init(codec->id);
  if (!parserCtx) {
    qDebug() << "av_parser_init error";
    return;
  }

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
  inLen = inFile.read(inData, IN_DATA_SIZE);
  while (inLen > 0) {

    /*在提交数据给解析器之前，要经过解析器解析，其内部核心逻辑是：ff_aac_ac3_parse 方法。
     *
     *   该方法的处理过程大概如下：
     *        将 inData 指向 pkt->data。
     *        将经过处理后的 inLen 赋值给 pkt->size。
     *
     *   该方法的返回值表示：the number of bytes of the input bitstream used.
     *
     * parse 函数的作用：
     *
     *   1. parser 在 context 初始化时被赋值，找到 parser 的函数是 ff_aac_ac3_parser。
     *   2. 不同解码器对单次解码数据的长度是有要求的，不能读多少就一次性交给解码器，所以先通过 parse 将缓冲区的数据切割成一帧一帧的数据，然后再交给解码器去解码。
     *   3. 下面用到了 packet，为什么不用调用 av_packet_unref 使用资源，因为这里 avPacket 内部的数据是指向的 inData 的，它自身没有分配缓冲区。
     */
    ret = av_parser_parse2(
        //上下文
        parserCtx, ctx,
        //【出参】预备的数据
        &pkt->data, &pkt->size,
        //【入参】读取到的数据
        (uint8_t *)inData, inLen,
        //其他固定参数
        AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

    if (ret < 0) {
      ERROR_BUF(ret);
      qDebug() << "av_parser_parse2 error" << errbuf;
      goto end;
    }

    qDebug() << "pkt->data" << (&pkt->data) << "inData" << (&inData);
    qDebug() << "pkt->size" << pkt->size << "inLen" << inLen;

    //跳过已经解析的数据
    inData += ret;
    //减去已经解析的数据的大小
    inLen -= ret;

    /*
     * 1. 如果 pkt->size，则说明还有数据需要解码，则进而调用 decode，否则就回去解析数据
     * 2. decode 返回 < 0 表示遇到错误，则结束解码过程。
     */
    if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
      goto end;
    }

    // 再次从文件读取数据进行解析的条件。
    //
    //   1. 已经解析的数据的缓冲区剩余量达到阈值。
    //   2. 没有读到文件末尾。
    //
    // 解析的时候为什么要设置阈值，有可能经过几次解析后，从文件中读取的数据中只剩余几个字节（inLen 变得很小），这时候给几个字节去 parse 和解码，可能会发生问题，比如剩余数据少于一帧的数据量。
    // 所以，这里不要等到 inLen == 0 时才再去从文件读取数据，而是达到已经阈值后就开始读，不过再读之前要将旧数据移动到缓冲区前面。
    if (inLen < REFILL_THRESHOLD && !inEnd) {
      //把解析未被解码的数据移动到缓冲区前面，以便读取的新数据接着未被解码的数据放置
      memmove(inDataArray, inData, inLen);

      //将以解析数据缓冲区指针移到前面
      inData = inDataArray;
      int len = inFile.read((inData + inLen), IN_DATA_SIZE - inLen);
      if (len > 0) {
        //更新已经读取的数据的长度
        inLen += len;
      } else {
        //标记，表示已经读到了文件末尾
        inEnd = 1;
      }
    }
  }

  //刷新缓冲区域（packet 传 null 即表示要刷新缓冲区）
  decode(ctx, nullptr, frame, outFile);

  // step6：赋值输出参数
  out.sampleFmt = ctx->sample_fmt;
  out.sampleRate = ctx->sample_rate;
  out.chLayout = ctx->channel_layout;

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
