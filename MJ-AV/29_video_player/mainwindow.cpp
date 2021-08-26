#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  _player = new VideoPlayer();
  initAll();
}

void MainWindow::initAll() {
  //监听播放器状态变更（更新UI）
  connect(_player, &VideoPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);
  //监听播放进度更新（更新UI）
  connect(_player, &VideoPlayer::timeChanged, this, &MainWindow::onPlayerTimeChanged);
  //监听播放器初始化完毕（更新UI）
  connect(_player, &VideoPlayer::initFinished, this, &MainWindow::onPlayerInitFinished);
  //监听播放器播放失败（更新UI）
  connect(_player, &VideoPlayer::playFailed, this, &MainWindow::onPlayerPlayFailed);

  //监听播放器解码出的一帧，进行渲染
  connect(_player, &VideoPlayer::frameDecoded, ui->videoWidget, &VideoWidget::onPlayerFrameDecoded);
  //监听播放器状态变更（播放组件）
  connect(_player, &VideoPlayer::stateChanged, ui->videoWidget, &VideoWidget::onPlayerStateChanged);

  //监听时间滑块的点击（设置进度）
  connect(ui->currentSlider, &VideoSlider::clicked, this, &MainWindow::onTimeSliderClicked);

  //设置音量滑块的范围
  ui->volumeSlider->setRange(VideoPlayer::Volume::MIN, VideoPlayer::Volume::MAX);
  //设置默认音量大小
  ui->volumeSlider->setValue(ui->volumeSlider->maximum() >> 3);
}

MainWindow::~MainWindow() {
  delete ui;
  delete _player;
}

void MainWindow::on_openFileButton_clicked() {

  QString filename = QFileDialog::getOpenFileName(
      //组件，可以传 nullptr
      nullptr,
      //标题
      "选择多媒体文件",
      //默认打开目录
      "D:/code/av/data/data02",
      //文件类型
      "视频文件 (*.mp4 *.avi *.mkv);;"
      "音频文件 (*.mp3 *.aac);;");

  qDebug() << "选择的文件" << filename;

  if (filename.isEmpty()) {
    return;
  }

  // 开始播放打开的文件
  _player->setFilename(filename);
  _player->play();
}

//===========================================================================
// 播放控制
//===========================================================================

void MainWindow::on_playButton_clicked() {
  VideoPlayer::State state = _player->getState();
  if (state == VideoPlayer::Playing) {
    _player->pause();
  } else {
    _player->play();
  }
}

void MainWindow::on_stopButton_clicked() {
  //停止
  _player->stop();
}

void MainWindow::on_muteButton_clicked() {
  if (_player->isMute()) {
    _player->setMute(false);
    ui->muteButton->setText("静音");
  } else {
    _player->setMute(true);
    ui->muteButton->setText("开音");
  }
}

void MainWindow::onTimeSliderClicked(VideoSlider *slider) {
  qDebug() << "进度" << slider->value();
  _player->setTime(slider->value());
}

void MainWindow::on_currentSlider_valueChanged(int value) {
  qDebug() << "进度" << value;
  ui->currentTimeabel->setText(getTimeText(value));
  // TODO：支持随意拖动来更新进度
  //_player->setTime(value);
}

void MainWindow::on_volumeSlider_valueChanged(int value) {
  qDebug() << "音量" << value;
  ui->volumeLable->setText(QString("%1").arg(value));
  _player->setVolumn(value);
}

//===========================================================================
// 被动根据播放状态更新UI。
//===========================================================================

void MainWindow::onPlayerInitFinished(VideoPlayer *player) {
  //初始化完毕
  //获取视频时长
  int duration = player->getDuration();
  //设置时间进度条范围
  ui->currentSlider->setRange(0, duration);
  //设置时间文字
  ui->totalTimeLable->setText(getTimeText(duration));
}

void MainWindow::onPlayerStateChanged(VideoPlayer *player) {
  qDebug() << "onPlayerStateChanged" << player->getState();

  //状态变更
  VideoPlayer::State state = player->getState();
  if (state == VideoPlayer::Playing) {
    ui->playButton->setText("暂停");
  } else {
    ui->playButton->setText("播放");
  }

  if (state == VideoPlayer::Stopped) {
    ui->playButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    ui->currentSlider->setEnabled(false);
    ui->volumeSlider->setEnabled(false);
    ui->muteButton->setEnabled(false);

    //重置播放进度
    ui->currentSlider->setValue(0);
    ui->currentTimeabel->setText(getTimeText(0));

    //显示打开文件界面
    ui->playWidget->setCurrentWidget(ui->openFilePage);
  } else {
    ui->playButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
    ui->currentSlider->setEnabled(true);
    ui->volumeSlider->setEnabled(true);
    ui->muteButton->setEnabled(true);

    // 显示播放视频的页面
    ui->playWidget->setCurrentWidget(ui->videoPage);
  }
}

void MainWindow::onPlayerPlayFailed(VideoPlayer *player) {
  //提醒播放错误
  QMessageBox::critical(nullptr, "提示", "哦豁，播放失败！");
}

//===========================================================================
// 被动根据播放进度更新UI。
//===========================================================================

void MainWindow::onPlayerTimeChanged(VideoPlayer *player) {
  //更新时间显示
  ui->currentSlider->setValue(player->getTime());
}

QString MainWindow::getTimeText(int value) {
  //方式1：
  //    int h = seconds / 3600;
  //    int m = (seconds % 3600) / 60;
  //    int m = (seconds / 60) % 60;
  //    int s = seconds % 60;

  //方式2：
  //    QString h = QString("0%1").arg(seconds / 3600).right(2);
  //    QString m = QString("0%1").arg((seconds / 60) % 60).right(2);
  //    QString s = QString("0%1").arg(seconds % 60).right(2);
  //    return QString("%1:%2:%3").arg(h).arg(m).arg(s);

  //方式3：
  QLatin1Char fill = QLatin1Char('0');
  return QString("%1:%2:%3").arg(value / 3600, 2, 10, fill).arg((value / 60) % 60, 2, 10, fill).arg(value % 60, 2, 10, fill);
}
