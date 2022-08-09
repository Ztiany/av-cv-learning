#include "mainwindow.h"
#include "playthread.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <SDL2/SDL.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void showVersion() {
  SDL_version version;
  SDL_VERSION(&version);
  qDebug() << version.major << version.minor << version.patch;
}

void MainWindow::startPlay() {
  this->_playThread = new PlayThread(this);
  this->_playThread->start();

  connect(this->_playThread, &PlayThread::finished, [this]() {
    this->ui->startPlay->setText("开始播放");
    this->_playThread = nullptr;
  });

  this->ui->startPlay->setText("停止播放");
}

void MainWindow::stopPlay() {
  this->_playThread->requestInterruption();
  this->ui->startPlay->setText("开始播放");
  this->_playThread = nullptr;
}

void MainWindow::on_startPlay_clicked() {
  if (this->_playThread) {
    stopPlay();
  } else {
    startPlay();
  }
}
