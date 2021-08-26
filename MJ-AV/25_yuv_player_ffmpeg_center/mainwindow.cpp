#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "yuvplayer.h"
#include <QDebug>

static int yuvIdx = 0;

// YUV 文件的描述信息
static Yuv yuvs[] = {
    //设置多个视频文件，自动轮流播放
    {"D:/code/av/data/data01/yuv420p_320x240.yuv", 320, 240, AV_PIX_FMT_YUV420P, 30},
    {"D:/code/av/data/data02/yuv420p_320x240.yuv", 320, 240, AV_PIX_FMT_YUV420P, 30}};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  //创建播放器【YUVPlayer也是一个 Widget】
  _player = new YUVPlayer(this);

  //监听播放器状态
  connect(_player, &YUVPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);

  //设置播放器的大小和位置
  int w = 300;                 //视频组件宽
  int h = 300;                 //视频组件高
  int x = (width() - w) >> 1;  //让组件水平居中
  int y = (height() - h) >> 1; //让组件垂直居中
  _player->setGeometry(x, y, w, h);

  _player->setYuv(yuvs[yuvIdx]);
}

MainWindow::~MainWindow() { delete ui; }

//开始播放
void MainWindow::on_start_clicked() {
  if (_player->isPlaying()) {
    _player->pause();
  } else {
    _player->play();
  }
}

//停止播放
void MainWindow::on_stop_clicked() { _player->stop(); }

//播放下一个
void MainWindow::on_next_clicked() {
  int yuvCount = sizeof(yuvs) / sizeof(Yuv);
  yuvIdx = ++yuvIdx % yuvCount;

  _player->stop();
  _player->setYuv(yuvs[yuvIdx]);
  _player->play();
}

void MainWindow::onPlayerStateChanged() {
  if (_player->getState() == YUVPlayer::Playing) {
    ui->start->setText("暂停");
  } else {
    ui->start->setText("播放");
  }
}
