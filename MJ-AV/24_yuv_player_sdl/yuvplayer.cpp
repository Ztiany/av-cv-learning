#include "yuvplayer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

// SDL 错误处理。
#define RET(judge, func)                                                                                                                                                                               \
  if (judge) {                                                                                                                                                                                         \
    qDebug() << #func << "error" << SDL_GetError();                                                                                                                                                    \
    return;                                                                                                                                                                                            \
  }

// SDL 与 FFmpeg 的像素格式映射。
static const std::map<AVPixelFormat, SDL_PixelFormatEnum> PIXEL_FORMAT_MAP = {
    // 420
    {AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
    // 422
    {AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2},
    // 未知
    {AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN}};

YUVPlayer::YUVPlayer(QWidget *parent) : QWidget(parent) {
  // 从 QT 组件创建 SDL 窗口
  _window = SDL_CreateWindowFrom((void *)winId());
  RET(!_window, SDL_CreateWindow);

  // 创建渲染上下文
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!_renderer) {
    _renderer = SDL_CreateRenderer(_window, -1, 0);
    RET(!_renderer, SDL_CreateRenderer);
  }
}

YUVPlayer::~YUVPlayer() {
  _file.close();
  SDL_DestroyTexture(_texture);
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
}

void YUVPlayer::play() {
  qDebug() << "play, fps = " << _yuv.fps << "_timerId =" << _timerId;
  // 开始一个定时器，事件间隔为 (1秒/帧率)，之后 timerEvent 讲被定期调用
  _timerId = startTimer(1000 / _yuv.fps);
  _state = YUVPlayer::Playing;
}

void YUVPlayer::pause() {
  if (_timerId) {
    killTimer(_timerId);
    _timerId = 0;
  }
  _state = YUVPlayer::Paused;
}

void YUVPlayer::stop() {
  if (_timerId) {
    killTimer(_timerId);
    _timerId = 0;
  }
  _state = YUVPlayer::Stopped;
}

bool YUVPlayer::isPlaying() { return _state == YUVPlayer::Playing; }

YUVPlayer::State YUVPlayer::getState() { return _state; }

void YUVPlayer::setYuv(Yuv &yuv) {
  _yuv = yuv;

  //创建纹理
  _texture = SDL_CreateTexture(
      //渲染上下文
      _renderer,
      //像素格式
      PIXEL_FORMAT_MAP.find(yuv.pixelFormat)->second,
      //数据形式
      SDL_TEXTUREACCESS_STREAMING,
      //宽高
      yuv.width, yuv.height);

  RET(!_texture, SDL_CreateTexture);

  // 打开文件
  _file.setFileName(yuv.filename);
  if (!_file.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << yuv.filename;
  }
}

void YUVPlayer::timerEvent(QTimerEvent *event) {

  // 图片大小
  int imgSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);

  char data[imgSize];
  if (_file.read(data, imgSize) > 0) {
    // 将YUV的像素数据填充到texture
    RET(SDL_UpdateTexture(_texture, nullptr, data, _yuv.width), SDL_UpdateTexture);

    // 设置绘制颜色（画笔颜色）
    RET(SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

    // 用绘制颜色（画笔颜色）清除渲染目标
    RET(SDL_RenderClear(_renderer), SDL_RenderClear);

    // 拷贝纹理数据到渲染目标（默认是window）
    RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr), SDL_RenderCopy);

    // 更新所有的渲染操作到屏幕上
    SDL_RenderPresent(_renderer);
  } else {
    // 文件数据已经读取完毕
    killTimer(_timerId);
  }
}
