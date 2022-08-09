#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

/** =========================================================================
使用 sdl 播放 YUV 视频
========================================================================= */

// sdl 定义了 main 方法，这里去掉它的定义，否则无法编译。
#undef main

int main(int argc, char *argv[]) {

  //初始化 SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    qDebug() << "SDL_Init error" << SDL_GetError();
    return 0;
  }

  QApplication a(argc, argv);

  MainWindow w;
  w.show();

  int ret = a.exec();

  //退出 SDL
  SDL_Quit();

  return ret;
}
