#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>

extern "C" {
//格式
#include <libavformat/avformat.h>
//工具类
#include <libavutil/avutil.h>
}

#ifdef Q_OS_WIN
#define FMT_NAME "dshow"
#define DEVICE_NAME                                                                                                                                                                                    \
  "audio=@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave_{E3818887-"                                                                                                                           \
  "13AE-453D-A18A-DDA52BE80DF3}"
#define FILENAME "D:/out.pcm"
#elif
#define FMT_NAME "avfoundation"
#define DEVICE_NAME ":0"
#define FILENAME "/Users/mj/Desktop/out.pcm"
#endif

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

//开始录音
void MainWindow::on_audioButton_clicked() {

  // step2：获取输入格式对象：dshow 这种设备，在代码中就用 AVInputFormat 表示。
  //【在 windows 平台用 ffmpeg -devices 列出设备驱动，一般就是 dshow】
  AVInputFormat *fmt = av_find_input_format(FMT_NAME);
  if (!fmt) {
    qDebug() << "获取输入格式对象失败" << FMT_NAME;
    return;
  }
  qDebug() << "得到 AVInputFormat：" << fmt;

  // step3：打开设备得到上下文【可以使用上下文操作设备】
  AVFormatContext *ctx = nullptr; //这里一定要初始化，否则 ctx 不会被赋值
  // ffmpeg -f dshow -list_devices true -i dumy 得到的名称
  // 这里的格式为 audio=device-name，具体参考
  // https://stackoverflow.com/questions/16618686/directshow-capture-source-and-ffmpeg
  const char *deviceName = DEVICE_NAME;
  int ret = avformat_open_input(&ctx, deviceName, fmt, nullptr);
  if (ret < 0) {
    char errorbuf[1024];
    av_strerror(ret, errorbuf, sizeof(errorbuf));
    qDebug() << "打开设备失败：ret = " << ret << " reason：" << errorbuf << " device name = " << deviceName;
    return;
  }

  qDebug() << "得到上下文：" << ctx;

  // step4：采集数据
  const char *filename = FILENAME;
  QFile file(filename);
  ret = file.open(QFile::WriteOnly); // WriteOnly: create or truncate.
  if (ret < 0) {
    qDebug() << "打开文件失败：" << filename;
    //释放资源
    avformat_close_input(&ctx);
    return;
  }
  qDebug() << "开始录音";
  AVPacket pkt; //用于存放数据
  int count = 50;
  while (count-- > 0 && av_read_frame(ctx, &pkt) == 0) { //循环采集
    file.write((const char *)pkt.data, pkt.size);        //写入文件
  }

  // step5: 释放资源
  qDebug() << "录音结束";
  file.close();
  avformat_close_input(&ctx);
}
