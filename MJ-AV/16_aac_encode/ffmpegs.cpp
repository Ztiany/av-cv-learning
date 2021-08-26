#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

/* =========================================================================
 注意事项：
     1. AAC 编码时，缓冲区的大小应该为样帧的整数倍，因为总不能拿半个样本去编码，给 AVFrame 设置一些参数，
 比如 frame->nb_samples = ctx.frame_size，设置后，frame 缓冲区大小由 ctx->frame_size 决定。
【frame_size 是内置的推荐大小】
     2. 同样，这里没有考虑多 planer 的情况。
========================================================================= */

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define ERROR_BUF(ret)                                                                                                                                                                                 \
  char errbuf[1024];                                                                                                                                                                                   \
  av_strerror(ret, errbuf, sizeof(errbuf));

FFmpegs::FFmpegs() {}

/**
 * @brief 检查采样格式
 * @param codec 编码器
 * @param sample_fmt 待编码音频采样格式
 * @return 1 表示支持，否则不支持。
 */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt) {
  // codec->sample_fmts 中保持了所有该编码器支持的格式
  const enum AVSampleFormat *p = codec->sample_fmts;
  // AV_SAMPLE_FMT_NONE 是官方定义的一个边界值，便于开发者遍历时防止越界。
  while (*p != AV_SAMPLE_FMT_NONE) {
    qDebug() << "check_sample_fmt() checking:" << av_get_sample_fmt_name(*p);
    if (*p == sample_fmt) {
      return 1;
    }
    p++;
  }
  return 0;
}

/**
 * @brief encode 编码
 * @param ctx 上下文
 * @param frame 帧（PCM）
 * @param packet 包（AAC）
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

void FFmpegs::aacEncode(AudioEncodeSpec &in, const char *outFilename) {
  qDebug() << "开始编码---------------------------------------------------------->";
  // step1：定义需要用到的变量
  QFile inFile(in.filename);     // PCM文件
  QFile outFile(outFilename);    // AAC文件
  int ret;                       //函数调用返回值
  AVCodec *codec = nullptr;      //编码器
  AVCodecContext *ctx = nullptr; //编码上下文
  AVFrame *frame = nullptr;      //存储编码前的数据（PCM）
  AVPacket *packet = nullptr;    //存储编码后的数据（AAC）

  // step2：获取编码器
  // codec = avcodec_find_encoder(AV_CODEC_ID_AAC);//这里找打的是默认的编码器
  codec = avcodec_find_encoder_by_name("libfdk_aac");
  if (!codec) {
    qDebug() << "encoder not found";
    return;
  }

  // step3：检测当前编码器是否支持对应的采样格式【不同的编码器对采样格式有不同的要求】
  // libfdk_aac 编码器对输入数据的要求：采样格式必须是 16 位整数。
  if (!check_sample_fmt(codec, in.sampleFmt)) {
    qDebug() << "unsupported sample format" << av_get_sample_fmt_name(in.sampleFmt);
    return;
  }

  // step4：根据编码器创建对应的编码上下文【函数后面的 3 说明该函数是第 3 版，查看之前的版本，发现确实是有被标为废弃】
  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    qDebug() << "avcodec_alloc_context3 error";
    return;
  }

  // step5：设置编码参数
  ctx->sample_fmt = in.sampleFmt;    //采样格式标识【必须】
  ctx->channel_layout = in.chLayout; //通道数标识【必须】
  ctx->sample_rate = in.sampleRate;  //采样率【必须】

  // ctx->bit_rate = 32000;               //比特率，这里是给一个建议值【可选】
  ctx->profile = FF_PROFILE_AAC_HE_V2; //这里是设置采样哪种 AAC 的标准【可选】

  // step6：打开编码器
  // options 用于给特定编码器传递其特有的参数，对于 fdk_aac 编码器支持的参数，具体参考 https://trac.ffmpeg.org/wiki/Encode/AAC
  // AVDictionary *options = nullptr;
  // av_dict_set(&options, "vbr", "5", 0); //当开启 vbr 时，上面设置的 ctx->bit_rate 将被忽略
  // ret = avcodec_open2(ctx, codec, &options);
  ret = avcodec_open2(ctx, codec, nullptr);
  if (ret < 0) {
    ERROR_BUF(ret);
    qDebug() << "avcodec_open2 error" << errbuf;
    goto end;
  }

  // step7：创建 Frame，并为其创建缓冲区。
  frame = av_frame_alloc();
  if (!frame) {
    qDebug() << "av_frame_alloc error";
    goto end;
  }

  frame->nb_samples = ctx->frame_size; // frame 缓冲区中的样本帧数量（由 ctx->frame_size 决定）
  frame->format = ctx->sample_fmt;
  frame->channel_layout = ctx->channel_layout;

  //利用 nb_samples、format、channel_layout
  ret = av_frame_get_buffer(frame, 0); //最后一个参数都是传 0
  if (ret) {
    ERROR_BUF(ret);
    qDebug() << "av_frame_get_buffer error" << errbuf;
    goto end;
  }

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

  // step10：读取数据到 frame 中，并进行编码
  while ((ret = inFile.read((char *)frame->data[0], frame->linesize[0])) > 0) {
    //从文件的中读取的数据不一定能够填满整个缓冲区，所以这里算一下真正读取了多少个样本帧
    if (ret < frame->linesize[0]) {
      int bytes = av_get_bytes_per_sample((AVSampleFormat)frame->format); //采样点大小
      int ch = av_get_channel_layout_nb_channels(frame->channel_layout);  //通道数
      //修改通道数，编码器就是根据这个值来读取数据的。
      frame->nb_samples = ret / (bytes * ch);
    }
    //核心：进行编码
    if (encode(ctx, frame, packet, outFile) < 0) {
      goto end;
    }
  }
  // 刷新缓冲区【将缓冲区中可能剩余的数据全部消耗完】
  // 参考 avcodec_send_frame 方法注释：Frame can be NULL, in which case it is considered a flush packet.
  encode(ctx, nullptr, packet, outFile);

// step11：释放资源
end:
  inFile.close();
  outFile.close();
  av_frame_free(&frame);
  av_packet_free(&packet);
  avcodec_free_context(&ctx);
  qDebug() << "编码结束---------------------------------------------------------->";
}
