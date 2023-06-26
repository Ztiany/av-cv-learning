#ifndef SENDER_H
#define SENDER_H

#include <QObject>

class Sender : public QObject {
    Q_OBJECT
public:
    explicit Sender(QObject *parent = nullptr);

//声明信号函数
signals:
    int exit(int n1, int n2);
    void exit2(int n1, int n2);
};

#endif // SENDER_H
