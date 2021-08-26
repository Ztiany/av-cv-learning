#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H
#include <QThread>

class PlayThread : public QThread {
  Q_OBJECT

private:
  void run();

  void *_winId; //一个 QT 的 Widget ID，可用其创建一个 SDL 窗口。

public:
  explicit PlayThread(void *winId, QObject *parent = nullptr);
  ~PlayThread();

signals:
};

#endif // PLAYTHREAD_H
