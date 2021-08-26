#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videoplayer.h"
#include "videoslider.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:

  /** 选择多媒体文件 */
  void on_openFileButton_clicked();

  /** 停止 */
  void on_stopButton_clicked();
  /** 播放与暂停 */
  void on_playButton_clicked();
  /** 静音 */
  void on_muteButton_clicked();

  /** 点击调节播放进度 */
  void onTimeSliderClicked(VideoSlider *slider);
  /** 拖动调节音量 */
  void on_volumeSlider_valueChanged(int value);
  /** 拖动调节播放进度 */
  void on_currentSlider_valueChanged(int value);

  /** 播放初始化完毕 */
  void onPlayerInitFinished(VideoPlayer *player);
  /** 播放状态更新 */
  void onPlayerStateChanged(VideoPlayer *player);
  /** 播放失败 */
  void onPlayerPlayFailed(VideoPlayer *player);
  /** 播放的时间更新 */
  void onPlayerTimeChanged(VideoPlayer *player);

private:
  Ui::MainWindow *ui;
  VideoPlayer *_player;

  /** 初始化所有设置 */
  void initAll();

  /** 格式化时间 */
  QString getTimeText(int value);
};
#endif // MAINWINDOW_H
