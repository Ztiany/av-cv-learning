#include "mainwindow.h"

#include <QApplication>

//因为 SDL 系统重新定义了 main 方法，这里要解除。
#undef main

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
