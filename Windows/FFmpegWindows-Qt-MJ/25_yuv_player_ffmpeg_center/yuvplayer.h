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
  // YUV 视频文件
  const char *filename;
  // 宽高
  int width;
  int height;
  // 像素格式使用 FFmpeg 的更通用，因为渲染模块不一定就只能用 SDL
  AVPixelFormat pixelFormat;
  // 帧率
  int fps;
} Yuv;

class YUVPlayer : public QWidget {
  Q_OBJECT

public:
  explicit YUVPlayer(QWidget *parent = nullptr);
  ~YUVPlayer();

  /**播放状态*/
  typedef enum {
    /**停止*/
    Stopped = 0,
    /**播放中*/
    Playing,
    /**暂停*/
    Paused,
    /**完成*/
    Finished
  } State;

  void setYuv(Yuv &yuv);

  void play();
  void pause();
  void stop();

  bool isPlaying();

  State getState();

signals:

  /**
   * @brief stateChanged 信号函数，通知状态变更。
   */
  void stateChanged();

private:
  QImage *_currentImage = nullptr; //当前帧
  QRect _dstRect;                  //当前帧绘制位置
  QFile *_file = nullptr;          //打开的视频文件
  int _timerId = 0;                //定时器Id
  State _state = Stopped;          //当前状态
  bool _playing;                   //是否正在播放
  int _imgSize;                    //一帧图片的大小
  int _interval;                   //刷帧的时间

  //这里为什么不适用指针引用外部传入的 Yuv，因为外部的 Yuv 可能只是个临时对象，这里拷贝一份比较好。
  Yuv _yuv; // YUV 数据

  /**
   * 释放播放完的一帧
   */
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

  /**
   * @brief setState 更新播放状态
   * @param state
   */
  void setState(State state);

  /**
   * @brief closeFile 关闭当前文件
   */
  void closeFile();

  /**
   * @brief stopTimer 停止定时器
   */
  void stopTimer();
};

#endif // YUVPLAYER_H
