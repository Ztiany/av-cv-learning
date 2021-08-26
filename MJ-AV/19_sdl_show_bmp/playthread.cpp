#include "playthread.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h>

#define FILENAME "D:/code/av/data/data01/in.bmp"

//封装 SDL 函数调用的解决判断，如果调用失败则直接进入 end。
#define END(judge, func)                                                                                                                                                                               \
  if (judge) {                                                                                                                                                                                         \
    qDebug() << #func << "error" << SDL_GetError();                                                                                                                                                    \
    goto end;                                                                                                                                                                                          \
  }

PlayThread::PlayThread(QObject *parent) : QThread(parent) { connect(this, &PlayThread::finished, this, &PlayThread::deleteLater); }

PlayThread::~PlayThread() {
  disconnect();
  requestInterruption();
  quit();
  wait();
  qDebug() << this << "析构了";
}

void PlayThread::run() {
  // step1：定义相关变量
  SDL_Surface *surface = nullptr;      //像素数据
  SDL_Window *window = nullptr;        //窗口【用于展示图像】
  SDL_Renderer *render = nullptr;      //渲染上下文【理解为画笔】
  SDL_Texture *texture = nullptr;      //纹理【可以在纹理上进行绘制】
  SDL_Rect srcRect = {0, 0, 512, 512}; //矩形框【控制源图象】
  SDL_Rect dstRect = {0, 0, 512, 512}; //矩形框【控制目标图象】
  SDL_Rect rect;

  // step2：初始化 SDL 系统
  END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

  // step3：加载 bmp 图片
  surface = SDL_LoadBMP(FILENAME);
  END(!surface, SDL_LoadBMP);

  // step4：创建窗口
  window = SDL_CreateWindow(
      //标题
      "SDL展示BMP图片",
      //窗口原点
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      // 宽高
      surface->w, surface->h,
      //固定值
      SDL_WINDOW_SHOWN);
  END(!window, SDL_CreateWindow);

  // step5：创建上下文【render 的默认绘制对象是窗口】
  render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!render) { //如果不支持硬件加速
    render = SDL_CreateRenderer(window, -1, 0);
  }
  END(!render, SDL_CreateRenderer);

  // step5：创建纹理
  texture = SDL_CreateTextureFromSurface(render, surface);
  END(!texture, SDL_CreateTextureFromSurface);

  // step6：清除渲染目标
  END(SDL_SetRenderDrawColor(render, 255, 255, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
  // 用绘制颜色（画笔颜色）清除渲染目标
  END(SDL_RenderClear(render), SDL_RenderClear);

  // step7：拷贝纹理数据到渲染目标（默认是window）
  END(SDL_RenderCopy(render, texture, &srcRect, &dstRect), SDL_RenderCopy);

  // 测试：画一个红色框
  END(SDL_SetRenderDrawColor(render, 255, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColo r); // 设置绘制颜色（画笔颜色）
  rect = {0, 0, 50, 50};
  END(SDL_RenderFillRect(render, &rect), SDL_RenderFillRect) //绘制

  // step8：更新所有的渲染操作到屏幕上
  SDL_RenderPresent(render);

  // step9：等一下再停止线程
  SDL_Delay(2000);
end:
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(render);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
