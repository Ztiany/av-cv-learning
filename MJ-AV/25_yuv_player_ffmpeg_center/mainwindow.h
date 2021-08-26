#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "yuvplayer.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_start_clicked();

  void on_stop_clicked();

  void on_next_clicked();

  /**当播放器状态改变*/
  void onPlayerStateChanged();

private:
  Ui::MainWindow *ui;

  YUVPlayer *_player = nullptr;
};

#endif // MAINWINDOW_H
