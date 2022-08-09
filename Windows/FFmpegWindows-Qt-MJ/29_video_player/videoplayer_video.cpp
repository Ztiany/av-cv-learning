#include "videoplayer.h"
#include <QDebug>
#include <thread>
extern "C" {
#include <libavutil/imgutils.h>
}

int VideoPlayer::initVideoInfo() {
  //初始化视频解码器
  int ret = initDecoder(&_vDecodeCtx, &_vStream, AVMEDIA_TYPE_VIDEO);
  PUT_ERROR_AND_RETURN_RET(initDecoder);

  //初始化像素格式转换组件
  ret = initVideoSws();
  PUT_ERROR_AND_RETURN_RET(initVideoSws);

  return 0;
}

int VideoPlayer::initVideoSws() {
  //视频的原始宽高
  int inW = _vDecodeCtx->width;
  int inH = _vDecodeCtx->height;

  //输出 frame 的参数（输出宽高要求为 16 的倍数）
  _vSwsOutSpec.width = inW >> 4 << 4;  //调整为 16 的倍数
  _vSwsOutSpec.height = inH >> 4 << 4; //调整为 16 的倍数
  _vSwsOutSpec.pixFmt = AV_PIX_FMT_RGB24;
  _vSwsOutSpec.size = av_image_get_buffer_size(_vSwsOutSpec.pixFmt, _vSwsOutSpec.width, _vSwsOutSpec.height, 1);

  qDebug() << "initVideoSws"
           << "_vSwsOutSpec.width" << _vSwsOutSpec.width << "_vSwsOutSpec.height" << _vSwsOutSpec.height << "_vSwsOutSpec.pixFmt" << _vSwsOutSpec.pixFmt << "_vSwsOutSpec.size" << _vSwsOutSpec.size;

  //初始化像素格式转换上下文
  _vSwsCtx = sws_getContext(
      //输入格式
      inW, inH, _vDecodeCtx->pix_fmt,
      //输出格式
      _vSwsOutSpec.width, _vSwsOutSpec.height, _vSwsOutSpec.pixFmt,
      // flags specify which algorithm and options to use for rescaling
      SWS_BILINEAR,
      //其他参数
      nullptr, nullptr, nullptr);

  if (!_vSwsCtx) {
    qDebug() << "sws_getContext error";
    return -1;
  }

  // 初始化像素格式转换的输入 frame
  _vSwsInFrame = av_frame_alloc();
  if (!_vSwsInFrame) {
    qDebug() << "av_frame_alloc error";
    return -1;
  }

  _vSwsOutFrame = av_frame_alloc();
  if (!_vSwsOutFrame) {
    qDebug() << "av_frame_alloc error";
    return -1;
  }

  // 初始化 _vSwsOutFrame 的 data[0] 指向的内存空间（用于存储转换后的视频帧）
  int ret = av_image_alloc(_vSwsOutFrame->data, _vSwsOutFrame->linesize, _vSwsOutSpec.width, _vSwsOutSpec.height, _vSwsOutSpec.pixFmt, 1);
  PUT_ERROR_AND_RETURN_RET(av_image_alloc);

  qDebug() << "initVideoSws"
           << "_vSwsOutFrame.data[0]" << _vSwsOutFrame->data[0] << "_vSwsOutFrame.linesize[0]" << _vSwsOutFrame->linesize[0];

  return 0;
}

void VideoPlayer::addVideoPacket(AVPacket &packet) {
  _vMutex.lock();
  _vPacketList.push_back(packet);
  _vMutex.signal();
  _vMutex.unlock();
}

void VideoPlayer::clearVideoPacketList() {
  _vMutex.lock();
  for (AVPacket &pkt : _vPacketList) {
    av_packet_unref(&pkt);
  }
  _vPacketList.clear();
  _vMutex.unlock();
}

/**
 * 视频解码运行在单独的线程
 */
void VideoPlayer::decodeVideo() {
  //死循环，不断地解码视频包
  while (true) {

    //=============================
    // 处理暂停
    //=============================
    // 如果是暂停，并且没有 Seek 操作（暂停状态允许 seek 操作）
    if (_state == Paused && _vSeekTime == -1) {
      continue;
    }

    //=============================
    // 处理停止
    //=============================
    if (_state == Stopped) {
      _vCanFree = true; //告知视频组件可以安全释放了，否则如果音频解码线程还在用就释放的话，就会友异常。
      break;
    }

    //=============================
    // 取出一个视频包
    //=============================
    _vMutex.lock();
    if (_vPacketList.empty()) {
      _vMutex.unlock();
      continue; //没有包就结束此次循环
    }
    // 取出头部的视频包
    AVPacket pkt = _vPacketList.front();
    _vPacketList.pop_front();
    _vMutex.unlock();

    //=============================
    // 音视频同步处理（1）拿到视频时间戳
    //=============================
    if (pkt.dts != AV_NOPTS_VALUE) {
      _vTime = av_q2d(_vStream->time_base) * pkt.dts;
      qDebug() << "取出视频包"
               << "_vTime" << _vTime;
    }

    //=============================
    // 开始视频解码
    //=============================
    int ret = avcodec_send_packet(_vDecodeCtx, &pkt); //发生包
    av_packet_unref(&pkt);
    PUT_ERROR_AND_CONTINUE(avcodec_send_packet);

    while (true) {
      ret = avcodec_receive_frame(_vDecodeCtx, _vSwsInFrame); //接收帧
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      } else {
        PUT_ERROR_AND_BREAK(avcodec_receive_frame);
      }

      //=============================
      // seek 处理
      //=============================
      /*
       * （1）真正 seek 到的时间会比期望 seek 的时间提前一些，因为要定位到前面的 I 帧。
       * （2）对于视频，一定要在解码成功后（即 avcodec_receive_frame 函数之后），再进行下面的判断【不能先释放，万一使用了 I 帧，会导致 B、P 帧解码失败，从而出现视频撕裂】
       * （3）这里发现视频的时间是早于 seekTime 的，直接丢弃，为了 防止往后 seek 时，刚开始的视频帧跳动非常快，因为可能 seek 时，正好已经将之前的 pkt
       *      送到了解码器，而解码出来后，由于那几帧的时间远远小于音频时间戳，则会非常快速的渲染【即不会被下面的 while 循环等待】。
       */
      if (_vSeekTime >= 0) {
        if (_vTime < _vSeekTime) {
          continue;
        } else {
          _vSeekTime = -1;
        }
      }

      //=============================
      // 像素格式转换
      //=============================
      int height = sws_scale(
          //上下文
          _vSwsCtx,
          //输入数据
          _vSwsInFrame->data, _vSwsInFrame->linesize, 0, _vDecodeCtx->height,
          //输出数据
          _vSwsOutFrame->data, _vSwsOutFrame->linesize);

      //=============================
      // 音视频同步处理（2）循环等待
      //=============================
      if (_hasAudio) {
        // 如果视频包过早被解码出来，那就需要等待对应的音频时钟到达
        while (_vTime > _aTime && _state == Playing /*如果是暂停状态，允许 seek，不需要等待音频时间戳*/) {
          // qDebug() << "等待同步"  << "_vTime" << _vTime << "_aTime" << _aTime;
        }
      } else {
        // TODO，没有音频的情况，此时视频应该自己等待。
      }

      //=============================
      // 发送数据到渲染组件
      //=============================
      uint8_t *data = (uint8_t *)av_malloc(_vSwsOutSpec.size); //申请同等大小的内存
      memcpy(data, _vSwsOutFrame->data[0], _vSwsOutSpec.size); //拷贝解码后的数据
      emit frameDecoded(this, data, _vSwsOutSpec);             //发出信号
      qDebug() << "渲染了一帧"
               << "_vTime" << _vTime << "_aTime" << _aTime;
    }
    //内循环结束
  }
}

void VideoPlayer::freeVideo() {
  clearVideoPacketList();             //清空视频包
  avcodec_free_context(&_vDecodeCtx); //释放视频解码上下文
  sws_freeContext(_vSwsCtx);          //释放视频格式转换上下文
  av_frame_free(&_vSwsInFrame);       //释放输入 Frame
  if (_vSwsOutFrame) {                //释放输出 Frame（data 是自己申请的，需要自己释放）
    av_freep(&_vSwsOutFrame->data[0]);
    av_frame_free(&_vSwsOutFrame);
  }

  //重置视频相关变量
  _vSwsCtx = nullptr;
  _vStream = nullptr;
  _vTime = 0;
  _vCanFree = false;
  _vSeekTime = -1;
}
