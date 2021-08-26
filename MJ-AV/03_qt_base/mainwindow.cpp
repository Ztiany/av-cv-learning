#include "mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include "mypushbutton.h"

//MainWindow 的构造函数
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent) {

    qDebug() << this;
    qDebug() << (parent == nullptr);//MainWindow 没有父控件，可以认为它的父控件就是屏幕。
    qDebug() << "MainWindow被创建了";

    // 设置窗口标题
    setWindowTitle("主窗口");
    // 设置窗口大小
    //resize(600, 600);
    setFixedSize(600, 600);

    // 设置窗口位置
    move(100, 100);

    // 添加第1个按钮
    QPushButton *btn = new QPushButton;//在 QT 中，new 出来的对象不需要手动 delete，当父控件销毁时，子控件也会被销毁。
    btn->setText("登录");
    btn->setFixedSize(100, 30);
    btn->move(100, 200);
    btn->setParent(this);// 设置按钮的父控件，这样 btn 才能显示在父控件中。

    // 添加第2个按钮
    //new QPushButton("注册", this);
    QPushButton *btn2 = new MyPushButton(this);
    btn2->setText("注册");
}

//MainWindow 的析构函数
MainWindow::~MainWindow() {
    qDebug() << "MainWindow被销毁了";
}

