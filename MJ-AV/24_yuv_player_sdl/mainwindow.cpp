#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "yuvplayer.h"
#include <QDebug>
#include <SDL2/SDL.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  //创建播放器
  _player = new YUVPlayer(this);
  int w = 700;                 //视频组件宽
  int h = 400;                 //视频组件高
  int x = (width() - w) >> 2;  //让组件水平居中
  int y = (height() - h) >> 2; //让组件垂直居中
  _player->setGeometry(x, y, w, h);

  // YUV 文件的描述信息
  Yuv yuv = {//路径
             "D:/code/av/data/data01/yuv420p_320x240.yuv",
             //视频宽高
             320, 240,
             //格式
             AV_PIX_FMT_YUV420P,
             //帧率
             30};

  _player->setYuv(yuv);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_start_clicked() {
  if (_player->isPlaying()) {
    _player->pause();
    ui->start->setText("播放");
  } else {
    _player->play();
    ui->start->setText("暂停");
  }
}

void MainWindow::on_stop_clicked() { _player->stop(); }
