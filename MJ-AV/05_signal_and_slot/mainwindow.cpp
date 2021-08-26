#include "mainwindow.h"
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent) {

    QPushButton *btn = new QPushButton;
    btn->setText("关闭");
    btn->setFixedSize(100, 30);
    btn->setParent(this);

    // 连接信号与槽
    //  1，功能：点击按钮，关闭MainWindow窗口
    //  2. 实现：btn点击时发出信号，MainWindow会接收信号，然后调用槽函数：close
    connect(btn, &QPushButton::clicked, this, &MainWindow::close);

    //断开连接
    //disconnect(btn, &QPushButton::clicked, this, &MainWindow::close);
}

MainWindow::~MainWindow() {

}

