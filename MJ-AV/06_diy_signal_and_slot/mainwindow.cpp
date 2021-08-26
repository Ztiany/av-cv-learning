#include "mainwindow.h"
#include "sender.h"
#include "receiver.h"

#include <QDebug>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent) {

    QPushButton *btn = new QPushButton;
    btn->setText("按钮");
    btn->setFixedSize(100, 40);
    btn->setParent(this);

    //用 MainWindow 的 handleClick 槽处理点击
    connect(btn, &QPushButton::clicked, this, &MainWindow::handleClick);
    //使用 lambda 表示接收信号
    connect(btn, &QPushButton::clicked, []() {
           qDebug() << "点击了按钮";
    });

    //自定义信号与槽
    Sender *sender = new Sender;
    Receiver *receiver = new Receiver;


    //点击按钮时，发送一个信号，其实就是连接两个信号，这就要求两个信号的参数符合规范。
    //connect(btn, &QPushButton::clicked, sender, &Sender::exit);

    connect(sender, &Sender::exit, [](int n1, int n2) {
           qDebug() << "Lambda" << n1 << n2;
    });
    //发送信号
    emit sender->exit(10, 20);

    connect(sender, &Sender::exit, receiver, &Receiver::handleExit);
    connect(sender, &Sender::exit2, receiver, &Receiver::handleExit2);

// 2个信号循环连接，会导致死循环。
//    connect(sender, &Sender::exit, sender, &Sender::exit2);
//    connect(sender, &Sender::exit2, sender, &Sender::exit);
//    emit sender->exit2(10, 20);

    //信号处理有返回值
    qDebug() << "emit sender result: " << emit sender->exit(10, 20);
    emit sender->exit2(1, 3);

    //释放内存。
    delete sender;
    delete receiver;
}

void MainWindow::handleClick() {
    qDebug() << "点击了按钮 - handleClick";
}

MainWindow::~MainWindow() {
}

