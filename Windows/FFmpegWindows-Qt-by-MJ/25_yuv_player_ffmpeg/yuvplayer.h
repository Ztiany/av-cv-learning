#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QFile>
#include <QObject>
#include <QWidget>

extern "C" {
#include <libavutil/avutil.h>
}

/**
YUV 文件的描述信息
*/
typedef struct {
  const char *filename;
  int width;
  int height;
  //像素格式使用 FFmpeg 的更通用，因为渲染模块不一定就只能用 SDL
  AVPixelFormat pixelFormat;
  int fps;
} Yuv;

class YUVPlayer : public QWidget {
  Q_OBJECT

public:
  explicit YUVPlayer(QWidget *parent = nullptr);
  ~YUVPlayer();

  //播放状态
  typedef enum { Stopped = 0, Playing, Paused, Finished } State;

  void setYuv(Yuv &yuv);

  void play();
  void pause();
  void stop();
  bool isPlaying();

  State getState();

signals:

private:
  QImage *_currentImage = nullptr; //当前帧
  QFile _file;                     //路径

  int _timerId = 0;

  State _state = Stopped;
  bool _playing;

  //这里为什么不适用指针引用外部传入的 Yuv，因为外部的 Yuv 可能只是个临时对象，这里拷贝一份比较好。
  Yuv _yuv;

  void freeCurrentImage();

  /**
   * @brief timerEvent 定时器回调
   * @param event 事件对象
   */
  void timerEvent(QTimerEvent *event);

  /**
   * @brief YUVPlayer::paintEvent 每次刷新，都会调用这个方法。
   * @param event 事件
   */
  void paintEvent(QPaintEvent *event);
};

#endif // YUVPLAYER_H
