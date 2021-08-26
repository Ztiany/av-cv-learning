#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "videoplayer.h"
#include <QImage>
#include <QWidget>
/**
 * @brief 视频渲染组件
 */
class VideoWidget : public QWidget {
  Q_OBJECT
public:
  explicit VideoWidget(QWidget *parent = nullptr);
  ~VideoWidget();

public slots:

  /**
   * @brief 解码出了一帧，需要进行渲染。
   */
  void onPlayerFrameDecoded(VideoPlayer *player, uint8_t *data, VideoPlayer::VideoSwsSpec &spec);

  /**
   * @brief 播放状态发生变更。
   */
  void onPlayerStateChanged(VideoPlayer *player);

private:
  /**表示绘制的内容（RBG数据）*/
  QImage *_image = nullptr;

  /**控制绘制的位置*/
  QRect _rect;

  /**进行绘制*/
  void paintEvent(QPaintEvent *event) override;

  /**释放资源*/
  void freeImage();

signals:
};

#endif // VIDEOWIDGET_H
