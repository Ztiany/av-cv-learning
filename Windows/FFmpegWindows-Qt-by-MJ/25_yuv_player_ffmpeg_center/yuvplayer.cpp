#include "yuvplayer.h"
#include "ffmpegs.h"
#include <QDebug>
#include <QPainter>

/** =========================================================================
注意：不同于 SDL 原生支持 YUV 数据，要想在 Qt 的 Widget 上渲染图像，必须将 YUV 转换为 GRB
========================================================================= */

extern "C" {
#include <libavutil/imgutils.h>
}

YUVPlayer::YUVPlayer(QWidget *parent) : QWidget(parent) {
  // 设置背景色
  setAttribute(Qt::WA_StyledBackground, true); //允许自定义背景色【否则下面代码无效】
  setStyleSheet("background: black");          //设置自定义背景色
}

YUVPlayer::~YUVPlayer() {
  closeFile();
  freeCurrentImage();
  stopTimer();
}

void YUVPlayer::play() {
  if (!_file) {
    return;
  }

  if (getState() == Playing) {
    return;
  }

  // 开始一个定时器，事件间隔为 (1秒/帧率)，之后 timerEvent 讲被定期调用
  _timerId = startTimer(_interval);
  qDebug() << "play, fps = " << _yuv.fps << "_timerId =" << _timerId;

  // 更新状态
  setState(Playing);
}

void YUVPlayer::pause() {
  if (getState() != Playing) {
    return;
  }

  stopTimer();
  setState(Paused);
}

void YUVPlayer::stop() {
  if (getState() == Stopped) {
    return;
  }

  //停止计时器
  stopTimer();

  //释放当前帧
  freeCurrentImage();

  //刷新，变为黑色
  update();

  //更新状态
  setState(Stopped);
}

bool YUVPlayer::isPlaying() { return getState() == YUVPlayer::Playing; }

YUVPlayer::State YUVPlayer::getState() { return _state; }

void YUVPlayer::setState(State state) {
  if (state == _state) {
    return;
  }

  if (state == Stopped || state == Finished) {
    // 让文件读取指针回到文件首部
    _file->seek(0);
  }

  _state = state;

  emit stateChanged();
}

void YUVPlayer::freeCurrentImage() {
  if (!_currentImage) {
    return;
  }
  free(_currentImage->bits());
  delete _currentImage;
  _currentImage = nullptr;
}

void YUVPlayer::closeFile() {
  if (!_file) {
    return;
  }

  _file->close();
  delete _file;
  _file = nullptr;
}

void YUVPlayer::stopTimer() {
  if (_timerId == 0) {
    return;
  }

  killTimer(_timerId);
  _timerId = 0;
}

void YUVPlayer::setYuv(Yuv &yuv) {
  _yuv = yuv;
  qDebug() << "将要播放" << _yuv.filename;

  //关闭上一个文件
  closeFile();

  // 打开新文件
  _file = new QFile(_yuv.filename);
  if (!_file->open(QFile::ReadOnly)) {
    qDebug() << "file open error" << yuv.filename;
  }

  //根据帧率计算刷新时间
  _interval = 1000 / _yuv.fps;
  qDebug() << "刷新时间" << _interval;

  //根据像素格式、宽高得到用于存储该图片大小的字节数
  _imgSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);
  qDebug() << "图片大小" << _imgSize;

  //播放组件的大小
  int w = width();
  int h = height();

  //视频应该播放的位置
  int dx = 0;
  int dy = 0;
  int dw = _yuv.width;
  int dh = _yuv.height;

  //等比缩放
  if (dw > w || dh > h) { //需要对视频进行缩放
    if (dw * h > w * dh) {
      // 视频的宽高比 > 播放器的宽高比【对视频的宽进行缩放】
      dh = dh * w / dw;
      dw = w;
    } else {
      // 视频的宽高比 < 播放器的宽高比【对视频的高进行缩放】
      dw = dw * h / dh;
      dh = h;
    }
  }

  //居中
  dx = (w - dw) >> 1;
  dy = (h - dh) >> 1;

  //保存计算的结果
  _dstRect = QRect(dx, dy, dw, dh);

  qDebug() << "视频的矩形框" << dx << dy << dw << dh;
}

void YUVPlayer::paintEvent(QPaintEvent *event) {
  if (!_currentImage) {
    return;
  }

  // 将图片绘制到当前组件上
  QPainter(this).drawImage(_dstRect, *_currentImage);
}

void YUVPlayer::timerEvent(QTimerEvent *event) {

  // 读取的图片数据
  char data[_imgSize];

  if (_file->read(data, _imgSize) == _imgSize) {

    //输入/输出规格
    RawVideoFrame in = {data, _yuv.width, _yuv.height, _yuv.pixelFormat};
    RawVideoFrame out = {nullptr, _yuv.width, _yuv.height, AV_PIX_FMT_RGB24};

    //将输入格式转为 RGB
    FFmpegs::convertRawVideo(in, out);

    //释放之前的图片
    freeCurrentImage();

    //将转换后的图片封装成 Qt 中的 QImage
    _currentImage = new QImage((uchar *)out.pixels, out.width, out.height, QImage::Format_RGB888);

    //刷新，会触发 paintEvent 函数的调用
    update();

  } else {
    stopTimer();
    setState(Finished);
  }
}
