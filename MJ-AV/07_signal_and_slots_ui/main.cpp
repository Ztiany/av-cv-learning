#include "mainwindow.h"

#include <QApplication>

/** =========================================================================
 QT 基础编程：自定义信号与槽与 ui 文件【创建项目时，勾选 generate form】：

    1. window 解析 ui 时，会自定解析 ui 文件中的信号并绑定到槽。
    2. 自动绑定的原理是根据约定好的命名规则进行绑定的。
========================================================================= */
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  return a.exec();
}
