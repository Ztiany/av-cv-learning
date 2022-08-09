#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

//开始录音
void MainWindow::startRecord() {
  _audioThread = new AudioThread(this);
  _audioThread->start();

  // 监听线程结束
  connect(_audioThread, &AudioThread::finished, [this]() {
    _audioThread = nullptr;
    ui->audioButton->setText("开始录音");
  });
  ui->audioButton->setText("结束录音");
}

void MainWindow::stopRecord() {

  // 对应方案1
  //_audioThread->setStop(true);

  // 对应方案2
  _audioThread->requestInterruption();

  // ui操作
  _audioThread = nullptr;
  ui->audioButton->setText("开始录音");
}

void MainWindow::on_audioButton_clicked() {
  if (!_audioThread) { // 点击了“开始录音”
    // 开启线程【传入 this，绑定到 window，window 关闭时，释放线程】。
    startRecord();
  } else { // 点击了“结束录音”
    stopRecord();
  }
}
