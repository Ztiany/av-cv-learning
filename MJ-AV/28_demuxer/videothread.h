#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QObject>
#include <QThread>

class VideoThread : public QThread {
  Q_OBJECT

private:
  void run();

public:
  explicit VideoThread(QObject *parent = nullptr);
  ~VideoThread();

signals:
};

#endif // AUDIOTHREAD_H
