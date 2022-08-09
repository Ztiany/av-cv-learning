#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H
#include <QThread>

class AudioThread : public QThread {
  Q_OBJECT;

private:
  void run() override;
  volatile bool stop;

public:
  AudioThread(QObject *obj = nullptr);
  ~AudioThread();
  void setStop(bool stop);
};

#endif // AUDIOTHREAD_H
