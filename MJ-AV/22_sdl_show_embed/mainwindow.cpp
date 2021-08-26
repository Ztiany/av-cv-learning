#include "mainwindow.h"
#include "playthread.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <SDL2/SDL.h>
#include <playthread.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  _widget = new QWidget(this);
  _widget->setGeometry(200, 50, 512, 512);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_show_clicked() {
  PlayThread *playThread = new PlayThread((void *)_widget->winId(), this);
  playThread->start();
}
