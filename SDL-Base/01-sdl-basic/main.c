#include <stdio.h>

#include <SDL.h>

#undef main
int main()
{
    printf("Hello World!\n");

    SDL_Window *window = NULL;      // 声明窗口

    SDL_Init(SDL_INIT_VIDEO);       // 初始化SDL
    // 创建SDL Window
    window = SDL_CreateWindow("Basic Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640,
                              480,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) // 检测是否创建成功
    {
        printf("Can't create window, err:%s\n", SDL_GetError());
        return 1;
    }

    SDL_Delay(10000);  // 延迟10000ms

    SDL_DestroyWindow(window); // 消耗窗口

    SDL_Quit(); // 释放资源

    return 0;
}
