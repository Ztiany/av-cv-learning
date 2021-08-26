#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

//开始录音
void MainWindow::startRecord() {
  // 开启线程【传入 this，绑定到 window，window 关闭时，释放线程】。
  _audioThread = new AudioThread(this);
  _audioThread->start();

  // 监听线程结束时，更新 UI。

  connect(_audioThread, &AudioThread::finished,
          // lambda 表达式要访问外部变量的话，就要提前捕获 this，对应的语法就是 [this](){}。
          [this]() {
            _audioThread = nullptr;
            ui->audioButton->setText("开始录音");
          });

  // 更新 UI
  ui->audioButton->setText("结束录音");
}

void MainWindow::stopRecord() {
  // 请求线程结束
  _audioThread->requestInterruption();

  // ui操作
  _audioThread = nullptr;
  ui->audioButton->setText("开始录音");
}

void MainWindow::on_audioButton_clicked() {
  if (!_audioThread) { // 点击了“开始录音”
    startRecord();
  } else { // 点击了“结束录音”
    stopRecord();
  }
}
