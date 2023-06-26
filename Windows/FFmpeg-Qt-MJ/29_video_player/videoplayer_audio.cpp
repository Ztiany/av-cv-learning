#include "videoplayer.h"
#include <QDebug>

int VideoPlayer::initAudioInfo() {
  //初始化音频解码器
  int ret = initDecoder(&_aDecodeCtx, &_aStream, AVMEDIA_TYPE_AUDIO);
  PUT_ERROR_AND_RETURN_RET(initDecoder);

  //初始化音频重采样组件
  ret = initAudioSwr();
  PUT_ERROR_AND_RETURN_RET(initAudioSwr);

  // 初始化SDL
  ret = initSDL();
  PUT_ERROR_AND_RETURN_RET(initSDL);

  return 0;
}

int VideoPlayer::initAudioSwr() {
  //重采样输入参数
  _aSwrInSpec.sampleFmt = _aDecodeCtx->sample_fmt;
  _aSwrInSpec.sampleRate = _aDecodeCtx->sample_rate;
  _aSwrInSpec.chLayout = _aDecodeCtx->channel_layout;
  _aSwrInSpec.chs = _aDecodeCtx->channels;

  //重采样输出参数（用于交给 SDL 播放，sdl 不支持 fltp 格式的 pcm，于是要进行重采样。）
  _aSwrOutSpec.sampleFmt = AV_SAMPLE_FMT_S16;
  _aSwrOutSpec.sampleRate = 44100;
  _aSwrOutSpec.chLayout = AV_CH_LAYOUT_STEREO;
  _aSwrOutSpec.chs = av_get_channel_layout_nb_channels(_aSwrOutSpec.chLayout);
  _aSwrOutSpec.bytesPerSampleFrame = _aSwrOutSpec.chs * av_get_bytes_per_sample(_aSwrOutSpec.sampleFmt);

  //创建重采样上下文
  _aSwrCtx = swr_alloc_set_opts(
      //传空则表示由改函数初始化
      nullptr,
      //输出规格
      _aSwrOutSpec.chLayout, _aSwrOutSpec.sampleFmt, _aSwrOutSpec.sampleRate,
      //输出规格
      _aSwrInSpec.chLayout, _aSwrInSpec.sampleFmt, _aSwrInSpec.sampleRate,
      //其他
      0, nullptr);

  if (!_aSwrCtx) {
    qDebug() << "swr_alloc_set_opts error";
    return -1;
  }

  //初始化重采样上下文
  int ret = swr_init(_aSwrCtx);
  PUT_ERROR_AND_RETURN_RET(swr_init);

  //初始化重采样的输入 Frame
  _aSwrInFrame = av_frame_alloc();
  if (!_aSwrInFrame) {
    qDebug() << "av_frame_alloc error";
    return -1;
  }

  //初始化重采样输出 Frame
  _aSwrOutFrame = av_frame_alloc();
  if (!_aSwrOutFrame) {
    qDebug() << "av_frame_alloc error";
    return -1;
  }

  // 对于音频重采样的输出缓冲区，申请一个足够大的，从而避免每次都要动态计算。
  // 申请一个输出缓存区，并赋值给 _aSwrOutFrame->data。即 _aSwrOutFrame 的 data[0] 指向该内存空间。【这里申请了 4096，肯定比下面 SDL 设置的 512 要大】
  ret = av_samples_alloc(_aSwrOutFrame->data, _aSwrOutFrame->linesize, _aSwrOutSpec.chs, 4096, _aSwrOutSpec.sampleFmt, 1);
  PUT_ERROR_AND_RETURN_RET(av_samples_alloc);

  return 0;
}

int VideoPlayer::initSDL() {
  //音频参数
  SDL_AudioSpec spec;
  spec.freq = _aSwrOutSpec.sampleRate;  //采样率
  spec.format = AUDIO_S16LSB;           //采样格式
  spec.channels = _aSwrOutSpec.chs;     //通道数
  spec.samples = 512;                   //音频缓冲区的样本数量，影响 spec.callback 的第三个参数 len 大小（这个值必须是 2 的幂）
  spec.callback = sdlAudioCallbackFunc; //回调函数（由 SDL 调用，用于填充音频数据）
  spec.userdata = this;                 //传递给 sdlAudioCallbackFunc 函数的第一个参数。

  if (SDL_OpenAudio(&spec, nullptr)) {
    qDebug() << "SDL_OpenAudio error" << SDL_GetError();
    return -1;
  }
  return 0;
}

void VideoPlayer::sdlAudioCallbackFunc(void *userData, Uint8 *stream, int len) {
  VideoPlayer *player = (VideoPlayer *)userData;
  player->sdlAudioCallabck(stream, len);
}

void VideoPlayer::addAudioPacket(AVPacket &avPacket) {
  _aMutex.lock();
  _aPacketList.push_back(avPacket); //这里隐藏着 AVPacket 的拷贝操作
  //先 signal，再 unlock，防止出现【假性唤醒，参考 https://zhuanlan.zhihu.com/p/55123862】
  _aMutex.signal();
  _aMutex.unlock();
}

void VideoPlayer::clearAudioPacketList() {
  _aMutex.lock();
  for (AVPacket &pkt : _aPacketList) {
    av_packet_unref(&pkt);
  }
  _aPacketList.clear();
  _aMutex.unlock();
}

void VideoPlayer::sdlAudioCallabck(Uint8 *stream, int len) {
  //清空缓冲区中的数据
  SDL_memset(stream, 0, len);

  /*
   * len：SDL 音频缓冲区剩余大小（即还没有填充的大小）
   *
   * 这里考虑两种情况：
   *
   *     单次 decodeAudio() 返回的大小大于 SDL 需要的总大小，于是 PCM 还有剩余，下次回调时先将剩余的填充到 SDL 缓冲区中。
   *     单次 decodeAudio() 返回的大小小于 SDL 需要的总大小，于是一次 SDL 回调保护多次 decodeAudio 操作，直到填满为止。
   */
  while (len > 0 /*每次回调，尽量把 stream 填充满*/) {

    //=============================
    // 处理暂停
    //=============================
    if (_state == Paused) {
      break;
    }

    //=============================
    // 处理停止
    //=============================
    if (_state == Stopped) {
      _aCanFree = true; //告知音频组件可以安全释放了，否则如果音频解码线程还在用就释放的话，就会友异常。
      break;
    }

    //=============================
    // 音频数据解码
    //=============================

    // _aSwrOutIndex 记录的是 decodeAudio 解码出的 PCM 数据，有多少已经交给 SDL 处理了。
    // _aSwrOutIndex >= _aSwrOutSize 说明说明当前 PCM 的数据已经全部拷贝到 SDL 的音频缓冲区了，需要解码下一个 pkt，获取新的 PCM 数据
    if (_aSwrOutIndex >= _aSwrOutSize) {
      _aSwrOutSize = decodeAudio(); //_aSwrOutSize 记录 此次解码出的 PCM 大小
      _aSwrOutIndex = 0;            //重置 _aSwrOutIndex
      // 没有解码出PCM数据，那就静音处理（填充 0 即可）
      if (_aSwrOutSize <= 0) {
        _aSwrOutSize = 1024;
        memset(_aSwrOutFrame->data[0], 0, _aSwrOutSize);
      }
    }

    //计算此次填充了多少
    int fillLen = _aSwrOutSize - _aSwrOutIndex;
    fillLen = std::min(fillLen, len);

    //获取音量
    int volumn = _mute ? 0 : ((_volumn * 1.0 / MAX) * SDL_MIX_MAXVOLUME);

    //将数据复制到 SDL 缓冲区进行播放
    SDL_MixAudio(stream, _aSwrOutFrame->data[0] + _aSwrOutIndex, fillLen, volumn);

    //减去此次消耗的 PCM 大小
    len -= fillLen;
    stream += fillLen;
    _aSwrOutIndex += fillLen;
  }
}

int VideoPlayer::decodeAudio() {
  //=============================
  // 取出一个待解码音频包，需要保证线程安全。
  //=============================
  _aMutex.lock();
  if (_aPacketList.empty()) {
    _aMutex.unlock();
    return 0;
  }
  AVPacket pkt = _aPacketList.front();
  _aPacketList.pop_front();
  _aMutex.unlock();

  //=============================
  // 音视频同步（这里获取当前音频帧应当播放的时间）
  //=============================
  if (pkt.pts != AV_NOPTS_VALUE) {
    _aTime = av_q2d(_aStream->time_base) * pkt.pts; // 当前音频帧应当播放的时间
    emit timeChanged(this);                         // 通知外界：播放时间点发生了改变
  }

  //=============================
  // seek 操作
  //=============================
  if (_aSeekTime > 0) {
    //发现音频的时间是早于 seekTime 的，直接丢弃，【（1）配合 Video 解码对 seek 的处理；（2）优化音频，让其更接近 seek 到的时间】。
    if (_aTime < _aSeekTime) {
      av_packet_unref(&pkt); // 释放pkt
      return 0;
    } /*要 seek 到的时间小于当前时间*/ else {
      _aSeekTime = -1;
    }
  }

  //=============================
  // 音频解码
  //=============================
  int ret = avcodec_send_packet(_aDecodeCtx, &pkt); //送入解码器，进行解码
  av_packet_unref(&pkt);                            //释放 AVPacket
  PUT_ERROR_AND_RETURN_RET(avcodec_send_packet);

  ret = avcodec_receive_frame(_aDecodeCtx, _aSwrInFrame); //接收解码的 PCM
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {     //需要更多数据
    return 0;
  } else { //发生错误
    PUT_ERROR_AND_RETURN_RET(avcodec_receive_frame);
  }

  //=============================
  // 音频重采样（由于解码出来的PCM。跟SDL要求的PCM格式可能不一致）
  //=============================
  //计算重采样前对应的样本数在重采样后能产生多少个新的样本。
  int outSamples = av_rescale_rnd(
      //目标采样率
      _aSwrOutSpec.sampleRate,
      //原采样率和样本数
      _aSwrInFrame->nb_samples, _aSwrInSpec.sampleRate,
      //四舍五入
      AV_ROUND_UP);

  //执行重采样，返回值表示真正的样本数。
  ret = swr_convert(_aSwrCtx, _aSwrOutFrame->data, outSamples, (const uint8_t **)_aSwrInFrame->data, _aSwrInFrame->nb_samples);
  PUT_ERROR_AND_RETURN_RET(swr_convert);
  int samples = ret * _aSwrOutSpec.bytesPerSampleFrame;

  // qDebug() << "decodeAudio" << samples;
  return samples;
}

void VideoPlayer::freeAudio() {
  //重置所有音频相关的变量
  _aTime = 0;
  _aSwrOutIndex = 0;
  _aSwrOutSize = 0;
  _aStream = nullptr;
  _aCanFree = false;
  _aSeekTime = -1;

  //释放音频相关组件
  clearAudioPacketList();             //清空音频包
  avcodec_free_context(&_aDecodeCtx); //释放解码上下文
  swr_free(&_aSwrCtx);                //释放重采样上下文
  av_frame_free(&_aSwrInFrame);       //释放输入 Frame
  if (_aSwrOutFrame) {                //释放输出 Frame（因为 _aSwrOutFrame.data 是手动申请的，也需要手动释放）
    av_freep(&_aSwrOutFrame->data[0]);
    av_frame_free(&_aSwrOutFrame);
  }

  //停止 SDL 组件
  SDL_PauseAudio(1);
  SDL_CloseAudio();
}
