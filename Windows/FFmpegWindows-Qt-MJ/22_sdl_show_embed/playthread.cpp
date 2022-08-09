#include "playthread.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

/** =========================================================================
注意：由于 SDL 和 QT 的一些不兼容，会有一些问题，比如展示图片到 QT 组建后，如果 QT 窗口失去交单，则图片会消失。
========================================================================= */

//封装 SDL 函数调用的解决判断，如果调用失败则直接进入 end。
#define END(judge, func)                                                                                                                                                                               \
  if (judge) {                                                                                                                                                                                         \
    qDebug() << #func << "error" << SDL_GetError();                                                                                                                                                    \
    goto end;                                                                                                                                                                                          \
  }

#define FILENAME "D:/code/av/data/data01/yuv420p_512x512.yuv" //图片路径
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV                     //图片像素格式，这里表示是 YUV420 Planar
#define IMG_W 512                                             //图片宽
#define IMG_H 512                                             //图片高

PlayThread::PlayThread(void *winId, QObject *parent)
    : QThread(parent),
      //初始化 QT 的组件 ID。
      _winId(winId) {
  //断开连接
  connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
  disconnect();
  requestInterruption();
  quit();
  wait();
  qDebug() << this << "析构了";
}

void PlayThread::run() {
  // step1：定义相关变量
  SDL_Window *window = nullptr;   //窗口【用于展示图像】
  SDL_Renderer *render = nullptr; //渲染上下文【理解为画笔】
  SDL_Texture *texture = nullptr; //纹理【可以在纹理上进行绘制，内部是跟特定驱动程序相关的像素数据】
  QFile file(FILENAME);
  qDebug() << "step1：定义相关变量";

  // step2：初始化 SDL 系统
  END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);
  qDebug() << "step2：初始化 SDL 系统";

  // step3：创建窗口
  window = SDL_CreateWindowFrom(_winId);
  END(!window, SDL_CreateWindow);
  qDebug() << "step3：创建窗口";

  // step4：创建上下文【render 的默认绘制对象是窗口】
  render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!render) { //如果不支持硬件加速
    render = SDL_CreateRenderer(window, -1, 0);
  }
  END(!render, SDL_CreateRenderer);
  qDebug() << "step4：创建上下文【render 的默认绘制对象是窗口】";

  // step5：创建纹理
  texture = SDL_CreateTexture(
      //渲染上下文
      render,
      //像素格式
      PIXEL_FORMAT,
      // Changes frequently, lockable.
      SDL_TEXTUREACCESS_STREAMING,
      //宽高
      IMG_W, IMG_H);
  END(!texture, SDL_CreateTextureFromSurface);
  qDebug() << "step5：创建纹理";

  // step6：打开文件并读取 YUV 数据到纹理
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "file open error" << FILENAME;
    goto end;
  }
  SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W);
  qDebug() << "step6：打开文件并读取 YUV 数据到纹理";

  // step7：清除渲染目标
  END(SDL_SetRenderDrawColor(render, 255, 255, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
  // 用绘制颜色（画笔颜色）清除渲染目标
  END(SDL_RenderClear(render), SDL_RenderClear);
  qDebug() << "step7：清除渲染目标";

  // step8：拷贝纹理数据到渲染目标（默认是window）
  END(SDL_RenderCopy(render, texture, nullptr, nullptr), SDL_RenderCopy);
  qDebug() << "step8：拷贝纹理数据到渲染目标（默认是window）";

  // step9：更新所有的渲染操作到屏幕上
  SDL_RenderPresent(render);
  qDebug() << "step9：更新所有的渲染操作到屏幕上";

  // step10：等待退出事件
  while (!isInterruptionRequested()) {
    SDL_Event event;
    SDL_WaitEvent(&event);
    switch (event.type) {
    case SDL_QUIT:
      goto end;
    }
  }

end:
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(render);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
