#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 QT 基础编程：信号与槽
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
