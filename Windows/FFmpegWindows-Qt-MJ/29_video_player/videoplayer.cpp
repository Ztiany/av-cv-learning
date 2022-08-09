#include "videoplayer.h"
#include <QDebug>
#include <thread>

#define AUDIO_MAX_PKG_SIZE 1000
#define VIDEO_MAX_PKG_SIZE 500

VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent) {
  //让自定义的类型 VideoSwsSpec 可以成为信号函数的参数
  qRegisterMetaType<VideoPlayer::VideoSwsSpec>("VideoSwsSpec&");
  //初始化 Audio 子系统
  if (SDL_Init(SDL_INIT_AUDIO)) {
    // 返回值不是 0，就代表失败
    qDebug() << "SDL_Init error" << SDL_GetError();
    emit playFailed(this);
  }
}

VideoPlayer::~VideoPlayer() {
  //不再对外发射信号（防止 stop 时还去发射信号，而此时外界的 UI 组件已经销毁了。）
  disconnect();
  //停止播放
  stop();
  //推出 SDL 子系统
  SDL_Quit();
}

void VideoPlayer::play() {
  //如果是播放，就返回
  if (getState() == Playing) {
    return;
  }

  if (_state == Stopped) {
    std::thread([this]() {
      //开始读取文件并进行解封装
      readFile();
      // 调用 detach 的说明：https://stackoverflow.com/questions/22803600/when-should-i-use-stdthreaddetach
    }).detach();
  } else {
    //更新状态
    setState(Playing);
  }
}

void VideoPlayer::pause() {
  if (getState() != Playing) {
    return;
  }

  setState(Paused);
}

void VideoPlayer::stop() {
  if (getState() == Stopped) {
    return;
  }

  /*
   * 更新状态：不用 setState() 方法的原因：
   *    （1）因为希望先调用 free 后（即停止所有的线程，停止左右的解码工作后）再通知外界 UI 状态变更为 Stopped，以防止出现通知了外界 Stopped 了，而解码线程还在工作的情况。
   *    （2）free() 中有等待 Stopped 的循环，所以 free 一定要在状态为 Stopped 时调用。所以此处要做一下特殊处理。
   */
  _state = Stopped;

  //释放资源
  free();

  //手动通知外界
  emit stateChanged(this);
}

bool VideoPlayer::isPlaying() { return getState() == Playing; }

VideoPlayer::State VideoPlayer::getState() { return _state; }

void VideoPlayer::setState(State state) {
  if (state == _state) {
    return;
  }
  _state = state;
  emit stateChanged(this);
}

void VideoPlayer::setFilename(QString filename) {
  const char *name = filename.toStdString().c_str();
  //+1 是因为要一个字符集结束符，strlen 返回的长度不包含 '\0'。
  memcpy(_filename, name, strlen(name) + 1);
}

int VideoPlayer::getDuration() {
  // return _fmtCtx ? round(_fmtCtx->duration / 1000000.0) : 0;
  return _fmtCtx ? round(_fmtCtx->duration * av_q2d(AV_TIME_BASE_Q)) : 0;
}

int VideoPlayer::getTime() {
  // 以为音频的时间为准
  // TODO：处理没有音频的情况。
  return round(_aTime);
}

void VideoPlayer::setTime(int time) { _seekTime = time; }

void VideoPlayer::setVolumn(int volume) { _volumn = volume; }

int VideoPlayer::getVolumn() { return _volumn; }

void VideoPlayer::setMute(bool mute) { _mute = mute; }

bool VideoPlayer::isMute() { return _mute; }

void VideoPlayer::readFile() {
  qDebug() << "readFile";
  //定义返回结果
  int ret = 0;

  // FFmpeg 打开文件，创建封装上下文。
  qDebug() << "avformat_open_input" << _filename;
  ret = avformat_open_input(&_fmtCtx, _filename, nullptr, nullptr);
  // ret = avformat_open_input(&_fmtCtx, "D:/code/av/data/data02/1.mp4", nullptr, nullptr); // for debug
  PUT_ERROR_AND_TERMINATE(avformat_open_input);

  //检索上下文中的流信息
  ret = avformat_find_stream_info(_fmtCtx, nullptr);
  PUT_ERROR_AND_TERMINATE(avformat_find_stream_info);

  //输出媒体信息到控制台
  av_dump_format(_fmtCtx, 0, _filename, 0);
  fflush(stderr); //因为打印信息用到的是 error 标准输出，这里刷新以下，防止打印不全。

  //检测是否含有音视频流信息
  _hasAudio = initAudioInfo() >= 0; //初始化音频信息
  _hasVideo = initVideoInfo() >= 0; //初始化视频信息
  if (!_hasAudio && !_hasVideo) {
    fataError();
    return;
  }

  //执行到这里，说明初始化完毕，通知外界更新 UI。
  emit initFinished(this);
  //通知状态更改
  setState(Playing);

  //=============================
  // 启动音频线程（SDL），音频解码线程开始工作
  //=============================
  SDL_PauseAudio(0);
  //=============================
  // 启动视频线程，视频解码线程开始工作
  //=============================
  std::thread([this]() { decodeVideo(); }).detach();

  //=============================
  // 开始读文件，解封装，并加入到队列中
  //=============================
  AVPacket pkt;
  while (_state != Stopped) {

    //=============================
    // 处理 seek
    //=============================
    if (_seekTime >= 0) {
      int streamIndex;
      if (_hasAudio) { // 优先使用音频流索引
        streamIndex = _aStream->index;
      } else {
        streamIndex = _vStream->index;
      }

      //实现世界的时间 --> 播放时间戳
      AVRational timeBase = _fmtCtx->streams[streamIndex]->time_base;
      int64_t ts = _seekTime / av_q2d(timeBase);
      qDebug() << "do seek"
               << "_seekTime" << _seekTime << "ts" << ts;

      //调用 ffmpeg API，执行 seek。
      ret = av_seek_frame(_fmtCtx, streamIndex, ts, AVSEEK_FLAG_BACKWARD);

      if (ret < 0) { // seek 失败
        qDebug() << "seek失败" << _seekTime << ts << streamIndex;
        _seekTime = -1;
      } else {
        qDebug() << "seek成功" << _seekTime << ts << streamIndex;
        //记录音视频要跳到的时间，让各自线程对准 seek 到的时间
        //【防止往后 seek 时，刚开始的视频帧跳动非常快，因为可能 seek 时，正好已经将之前的 pkt 送到了解码器，而解码出来后，由于那几帧的时间小于音频时间戳。】
        _vSeekTime = _seekTime;
        _aSeekTime = _seekTime;
        _seekTime = -1;
        // 清空之前读取的数据包
        clearAudioPacketList();
        clearVideoPacketList();
        // 恢复时钟（防止音视频同步的判断条件卡住）
        _aTime = 0;
        _vTime = 0;
      }
    }

    //=============================
    // 检测队列的size，防止一次性将大文件全部读入内存中
    //=============================
    int vSize = _vPacketList.size();
    int aSize = _aPacketList.size();
    if (vSize >= VIDEO_MAX_PKG_SIZE || aSize >= AUDIO_MAX_PKG_SIZE) {
      continue;
    }

    //=============================
    // 开始解封装
    //=============================
    //读入一个 packet
    ret = av_read_frame(_fmtCtx, &pkt);
    if (ret == 0) {

      if (pkt.stream_index == _aStream->index) { //音频包
        addAudioPacket(pkt);
      } else if (pkt.stream_index == _vStream->index) { //视频包
        addVideoPacket(pkt);
      } else { //其他包则忽略
        //不是需要的pkt，直接释放
        av_packet_unref(&pkt);
      }
    } else if (ret == AVERROR_EOF) { //读到文件尾部
      if (vSize == 0 && aSize == 0) {
        _fmtCtxCanFree = true;
        break;
      }
    } else { //发生错误
      ERROR_BUFFER_FROM_RET;
      qDebug() << "av_read_frame error" << errorBuffer;
      continue;
    }
  } //读取文件结束

  //=============================
  // 收尾工作
  //=============================
  if (_fmtCtxCanFree) {
    // 走到这里，说明是文件正常播放完毕
    stop();
  } else {
    // 走到这里，说明是用户【点击停止】，所以标记一下：_fmtCtx 可以释放了。
    _fmtCtxCanFree = true;
  }
}

int VideoPlayer::initDecoder(AVCodecContext **decodeCtx, AVStream **stream, AVMediaType type) {
  //根据类型找到最合适的流信息
  int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
  PUT_ERROR_AND_RETURN_RET(av_find_best_stream);

  //根据索引找到流
  int streamIndex = ret;
  *stream = _fmtCtx->streams[streamIndex];
  if (!*stream) {
    qDebug() << "type" << type << "stream is empty";
    return -1;
  }

  //根据流类型找到合适的解码器
  AVCodec *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
  if (!decoder) {
    qDebug() << "type" << type << "decoder not found" << (*stream)->codecpar->codec_id;
    return -1;
  }

  //根据解码器来初始化解码上下文
  *decodeCtx = avcodec_alloc_context3(decoder);
  if (!decodeCtx) {
    qDebug() << "type" << type << "avcodec_alloc_context3 error";
    return -1;
  }

  //拷贝流的参数到解码上下文中【比如视频的宽、高、像素格式等】
  ret = avcodec_parameters_to_context(*decodeCtx, (*stream)->codecpar);
  PUT_ERROR_AND_RETURN_RET(avcodec_parameters_to_context);

  //打开解码器
  ret = avcodec_open2(*decodeCtx, decoder, nullptr);
  PUT_ERROR_AND_RETURN_RET(avcodec_open2);

  return 0;
}

void VideoPlayer::free() {
  while (_hasAudio && !_aCanFree) {
  };
  while (_hasVideo && !_vCanFree) {
  };
  while (!_fmtCtxCanFree) {
  };

  avformat_close_input(&_fmtCtx);
  _fmtCtxCanFree = false;
  _seekTime = -1;

  freeAudio();
  freeVideo();
}

void VideoPlayer::fataError() {
  //配合 stop 能够调用成功，防止已经是 Stop 状态从而导致 stop 无效。
  _state = Playing; //不要调用 setState()，这里仅仅是为了保证 stop 调用成功，不需要通知外界状态变更。
  //调用 stop
  stop();
  //通知发生错误
  emit playFailed(this);
}
