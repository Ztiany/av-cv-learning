#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H
#include <QThread>

class VideoThread : public QThread {
  Q_OBJECT;

private:
  void run() override;

public:
  VideoThread(QObject *obj = nullptr);
  ~VideoThread();
};

#endif // AUDIOTHREAD_H
