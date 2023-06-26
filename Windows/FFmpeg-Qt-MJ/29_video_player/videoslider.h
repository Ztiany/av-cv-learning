#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

#include <QSlider>

/**
 * @brief 进度条组件，允许点到什么地方，跳到什么地方。
 */
class VideoSlider : public QSlider {
  Q_OBJECT
public:
  explicit VideoSlider(QWidget *parent = nullptr);

signals:
  /** 信号：点击事件 */
  void clicked(VideoSlider *slider);

private:
  void mousePressEvent(QMouseEvent *event);
};
#endif // VIDEOSLIDER_H
