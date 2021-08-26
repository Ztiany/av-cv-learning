#include "demuxer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

/** =========================================================================
 使用 ffmpeg 将 mp4 解封装为 YUV 和 PCM

编码步骤：
    1.创建解封装上下文（打开输入文件，读取文件头）
        avformat_open_input
    2.检索流信息
        avformat_find_stream_info
    3.导出流信息到控制台
        av_dump_format
    4.初始化音频信息
        1> 初始化解码器
            a) 根据AVMEDIA_TYPE_AUDIO找到音频流，返回音频流的索引：av_find_best_stream
            b) 检查音频流是不是空的
            c) 为音频流找到合适的解码器：avcodec_find_decoder
            d) 创建解码上下文：avcodec_alloc_context3
            e) 从音频流中拷贝参数到解码上下文中：avcodec_parameters_to_context
            f) 打开解码器：avcodec_open2
        2> 打开音频的输出文件
        3> 保存音频信息
            a) 采样率
            b) 采样格式
            c) 声道布局
    5.初始化视频信息
        1> 初始化解码器
            a) 根据AVMEDIA_TYPE_VIDEO找到视频流，返回视频流的索引：av_find_best_stream
            b) 检查视频流是不是空的
            c) 为视频流找到合适的解码器：avcodec_find_decoder
            d) 创建解码上下文：avcodec_alloc_context3
            e) 从视频流中拷贝参数到解码上下文中：avcodec_parameters_to_context
            f) 打开解码器：avcodec_open2
        2> 打开视频的输出文件
        3> 保存视频信息
            a) 宽度、高度
            b) 像素格式
            c) 帧率
    6.初始化AVFrame、AVPacket
    7.从输入文件中读取数据，进行解码
    8.将解码后的数据写到文件中
    9.释放资源

对比程序运行的输出与命令行的输出，字节数完全一致，则说明程序正确，参考命令：

    抽取音频：ffmpeg -i 1.mp4 -vn -vcodec libaac -f f32le -ar 48000 -ac 2 demux_cmd.pcm
    抽取视频：ffmpeg -i .\1.mp4 demux_cmd.yuv
========================================================================= */

#define GET_AV_ERROR                                                                                                                                                                                   \
  char errorBuf[1024];                                                                                                                                                                                 \
  av_strerror(ret, errorBuf, sizeof(errorBuf));

#define END_IF_ERROR(func)                                                                                                                                                                             \
  if (ret < 0) {                                                                                                                                                                                       \
    GET_AV_ERROR;                                                                                                                                                                                      \
    qDebug() << #func << "error" << errorBuf;                                                                                                                                                          \
    goto end;                                                                                                                                                                                          \
  }

#define RETURN_IF_ERROR(func)                                                                                                                                                                          \
  if (ret < 0) {                                                                                                                                                                                       \
    GET_AV_ERROR;                                                                                                                                                                                      \
    qDebug() << #func << "error" << errorBuf;                                                                                                                                                          \
    return ret;                                                                                                                                                                                        \
  }

Demuxer::Demuxer() {}

void Demuxer::demux(const char *inFileanme, AudioDecodeSpec &aOut, VideoDecodeSpec &vOut) {
  // step1：保留参数，检索信息，定义必要的参数
  _aOut = &aOut;
  _vOut = &vOut;

  AVPacket *pkt = nullptr; //用于接收解封装后的数据。【h264/aac】
  int ret = 0;             //各种函数调用返回值

  //打开媒体文件，初始化格式上下文
  ret = avformat_open_input(&_fmtCtx, inFileanme, nullptr, nullptr);
  END_IF_ERROR(avformat_open_input);

  // 检索流信息：Read packets of a media file to get stream information.
  // This is useful for file formats with no headers such as MPEG.
  ret = avformat_find_stream_info(_fmtCtx, nullptr);
  END_IF_ERROR(avformat_find_stream_info);

  //输出流信息
  av_dump_format(_fmtCtx, 0, inFileanme, 0);
  fflush(stderr);

  // step2：初始化音视频信息
  ret = initAudioInfo();
  if (ret < 0) {
    goto end;
  }
  ret = initVideoInfo();
  if (ret < 0) {
    goto end;
  }

  // step3：初始化 AVFrame、AvPacket
  _currentFrame = av_frame_alloc();
  if (!_currentFrame) {
    qDebug() << "av_frame_alloc error";
    goto end;
  }

  pkt = av_packet_alloc();//ffmpeg4.4 av_package_init 过期，推荐使用 av_packet_alloc，将内存放在堆空间。
  pkt->data = nullptr;
  pkt->size = 0;

  //从输入文件中读取数据并解码， av_read_frame reutn 0 if OK,
  while (av_read_frame(_fmtCtx, pkt) == 0) {
    if (pkt->stream_index == _aStreamIndex) { // 读取到的是音频数据
      ret = decode(_aDecodeCtx, pkt, &Demuxer::writeAudioFrame);
    } else if (pkt->stream_index == _vStreamIndex) { // 读取到的是视频数据
      ret = decode(_vDecodeCtx, pkt, &Demuxer::writeVideoFrame);
    }

    // 释放pkt内部指针指向的一些额外内存
    av_packet_unref(pkt);

    //是否错误判断
    if (ret < 0) {
      goto end;
    }
  }

  //刷新缓冲区
  decode(_aDecodeCtx, nullptr, &Demuxer::writeAudioFrame);
  decode(_vDecodeCtx, nullptr, &Demuxer::writeVideoFrame);

end:
  //关闭文件
  _aOutFile.close();
  _vOutFile.close();
  //关闭解码器山下文
  avcodec_free_context(&_aDecodeCtx);
  avcodec_free_context(&_vDecodeCtx);
  //关闭格式上下文
  avformat_close_input(&_fmtCtx);
  //释放包和帧
  av_frame_free(&_currentFrame);
  av_packet_free(&pkt);
  av_freep(&_imageBuf[0]);
}

int Demuxer::initAudioInfo() {
  //初始化音频解码器
  int ret = initDecoder(&_aDecodeCtx, &_aStreamIndex, AVMEDIA_TYPE_AUDIO);
  RETURN_IF_ERROR(initDecoder);

  //打开音频输出文件
  _aOutFile.setFileName(_aOut->filename);
  if (!_aOutFile.open(QFile::WriteOnly)) {
    qDebug() << "file open error" << _aOut->filename;
    return -1;
  }

  // 保存音频参数
  _aOut->sampleRate = _aDecodeCtx->sample_rate;
  _aOut->sampleFmt = _aDecodeCtx->sample_fmt;
  _aOut->chLayout = _aDecodeCtx->channel_layout;

  qDebug() << "音频";
  qDebug() << "采样率" << _aOut->sampleRate;
  qDebug() << "通道数" << av_get_channel_layout_nb_channels(_aOut->chLayout);
  qDebug() << "采样格式" << av_get_sample_fmt_name(_aOut->sampleFmt);

  //初始化音频相关信息
  _sampleSize = av_get_bytes_per_sample(_aOut->sampleFmt); //每个采样点的大小
  _sampleFrameSize = _sampleSize * _aDecodeCtx->channels;  //每个采样帧的大小

  return 0;
}

int Demuxer::initVideoInfo() {
  //初始化音频解码器
  int ret = initDecoder(&_vDecodeCtx, &_vStreamIndex, AVMEDIA_TYPE_VIDEO);
  RETURN_IF_ERROR(initDecoder);

  //打开视频输出文件
  _vOutFile.setFileName(_vOut->filename);
  if (!_vOutFile.open(QFile::WriteOnly)) {
    qDebug() << "file open error" << _vOut->filename;
    return -1;
  }

  // 保存视频参数
  _vOut->width = _vDecodeCtx->width;
  _vOut->height = _vDecodeCtx->height;
  _vOut->pixFmt = _vDecodeCtx->pix_fmt;
  //解封装时，使用 av_guess_frame_rate 获取帧率
  AVRational framerate = av_guess_frame_rate(_fmtCtx, _fmtCtx->streams[_vStreamIndex], nullptr);
  _vOut->fps = framerate.num / framerate.den; // 帧率

  qDebug() << "视频";
  qDebug() << "宽" << _vOut->width;
  qDebug() << "高" << _vOut->height;
  qDebug() << "像素格式 int" << _vOut->pixFmt;
  qDebug() << "像素格式" << av_get_pix_fmt_name(_vOut->pixFmt);
  qDebug() << "帧率" << _vOut->fps;

  //创建用于存放一帧解码后图片的缓冲区，返回值为图片的大小。
  ret = av_image_alloc(_imageBuf, _imageLinesizes, _vOut->width, _vOut->height, _vOut->pixFmt, 1);
  RETURN_IF_ERROR(av_image_alloc);

  //获取图片的大小
  _imageSize = ret;

  return 0;
}

int Demuxer::initDecoder(AVCodecContext **decodeCtx, int *streamIndex, AVMediaType type) {
  // 根据type寻找最合适的流信息，返回的是索引值
  int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
  RETURN_IF_ERROR(av_find_best_stream);

  *streamIndex = ret;
  AVStream *stream = _fmtCtx->streams[ret];
  if (!stream) {
    qDebug() << "type:" << type << "stream is empty";
    return -1;
  }

  // 为当前流找到合适的解码器
  //    AVCodec *decoder = nullptr;
  //    if (stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
  //        decoder = avcodec_find_decoder_by_name("libfdk_aac");
  //    } else {
  //        decoder = avcodec_find_decoder(stream->codecpar->codec_id);
  //    }

  AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
  if (!decoder) {
    qDebug() << "avcodec_alloc_context3 error";
    return -1;
  }

  // 初始化解码上下文
  *decodeCtx = avcodec_alloc_context3(decoder);
  if (!decodeCtx) {
    qDebug() << "avcodec_alloc_context3 error";
    return -1;
  }

  // 从流中拷贝参数到解码上下文中【类似于之前的手动设置参数】
  ret = avcodec_parameters_to_context(*decodeCtx, stream->codecpar);
  RETURN_IF_ERROR(avcodec_parameters_to_context);

  // 打开解码器
  ret = avcodec_open2(*decodeCtx, decoder, nullptr);
  RETURN_IF_ERROR(avcodec_open2);

  return 0;
}

int Demuxer::decode(AVCodecContext *decodeCtx, AVPacket *pkt, void (Demuxer::*func)()) {
  //发生解封装后的数据到解码器
  int ret = avcodec_send_packet(decodeCtx, pkt);
  RETURN_IF_ERROR(avcodec_send_packet);

  //获取解码后的数据并写入文件
  while (true) {
    // 获取解码后的数据
    ret = avcodec_receive_frame(decodeCtx, _currentFrame);
    //看是否消耗了所有解码后的数据，如果是就要再回去解码。
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0;
    }

    //错误检测
    RETURN_IF_ERROR(avcodec_receive_frame);

    // 执行写入文件的代码
    (this->*func)();
  }
}

void Demuxer::writeVideoFrame() {
  // 拷贝 frame 的数据到 _imgBuf 缓冲区
  av_image_copy(
      //接收缓冲区，行 szie
      _imageBuf, _imageLinesizes,
      // frame 中的数据，行 szie
      (const uint8_t **)(_currentFrame->data), _currentFrame->linesize,
      //视频信息
      _vOut->pixFmt, _vOut->width, _vOut->height);

  // 将缓冲区的数据写入文件
  _vOutFile.write((char *)_imageBuf[0], _imageSize);
}

void Demuxer::writeAudioFrame() {
  // libfdk_aac 解码器，解码出来的 PCM 格式：s16
  // aac 解码器，解码出来的 PCM 格式：ftlp

  if (av_sample_fmt_is_planar(_aOut->sampleFmt)) { // 处理 planar
    /*
     * planar 是 FFMPEG 内部定义的存储格式，主要是方便开发者处理单声道数据。
     * 解码后如果是 planar 的数据（即非交错模式），则要处理成交错模式的。
     *
     * LLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRR【planar】
     *                   |
     *                   |
     *                  \|/
     * LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLR【非 planar】
     */
    // 外层循环：每一个声道的样本数【每个平面的 nb_samples 必然是一样的】
    // _currentFrame->data[0] 指向第一个 L
    // _currentFrame->data[1] 指向第一个 R
    for (int si = 0; si < _currentFrame->nb_samples; si++) {
      // 内层循环：有多少个声道
      for (int ci = 0; ci < _aDecodeCtx->channels; ci++) {
        char *begin = (char *)(_currentFrame->data[ci] + si * _sampleSize);
        _aOutFile.write(begin, _sampleSize);
      }
    }
  } else { // 非planar
    _aOutFile.write((char *)_currentFrame->data[0], _currentFrame->nb_samples * _sampleFrameSize);
  }
}
