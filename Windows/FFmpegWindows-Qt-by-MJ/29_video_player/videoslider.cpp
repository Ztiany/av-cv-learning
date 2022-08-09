#include "videoslider.h"
#include <QDebug>
#include <QMouseEvent>
#include <QStyle>

VideoSlider::VideoSlider(QWidget *parent) : QSlider(parent) { qDebug() << "VideoSlider"; }

void VideoSlider::mousePressEvent(QMouseEvent *event) {
  // 方式1：根据点击位置的x值，计算出对应的value
  // valueRange = max - min
  // value = min + (x / width) * valueRange
  // int value = minimum() + (event->pos().x() * 1.0 / width()) * (maximum() - minimum());
  // setValue(value);
  // QSlider::mousePressEvent(event); //表示接收事件

  // 方式2：sliderValueFromPosition
  int value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().x(), width());
  setValue(value);
  QSlider::mousePressEvent(event); //表示接收事件

  //发出信号，通知外面值被修改了。
  emit clicked(this);
}
