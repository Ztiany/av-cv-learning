#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videothread.h"
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
  void on_startConvert_clicked();

private:
  Ui::MainWindow *ui;
  VideoThread *_videoThread;
};
#endif // MAINWINDOW_H
