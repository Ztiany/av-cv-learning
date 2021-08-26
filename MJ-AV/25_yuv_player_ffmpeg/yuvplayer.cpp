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
  _file.close();
  freeCurrentImage();
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

  //重新初始化文件
  _file.close();
  _file.setFileName(_yuv.filename);
  if (!_file.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << _yuv.filename;
  }
}

bool YUVPlayer::isPlaying() { return _state == YUVPlayer::Playing; }

YUVPlayer::State YUVPlayer::getState() { return _state; }

void YUVPlayer::setYuv(Yuv &yuv) {
  _yuv = yuv;

  // 打开文件
  _file.setFileName(yuv.filename);
  if (!_file.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << yuv.filename;
  }
}

void YUVPlayer::paintEvent(QPaintEvent *event) {
  if (!_currentImage) {
    return;
  }

  // 将图片绘制到当前组件上
  QPainter(this).drawImage(QRect(0, 0, width(), height()), *_currentImage);
}

void YUVPlayer::timerEvent(QTimerEvent *event) {

  // 图片大小
  int imgSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);
  // 读取的图片数据
  char data[imgSize];

  if (_file.read(data, imgSize) > 0) {

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
    // 文件数据已经读取完毕
    stop();
  }
}

void YUVPlayer::freeCurrentImage() {
  if (!_currentImage) {
    return;
  }
  free(_currentImage->bits());
  delete _currentImage;
  _currentImage = nullptr;
}
