#include <SDL.h>
#include <stdio.h>

SDL_mutex *s_lock = NULL;
SDL_cond *s_cond = NULL;

int thread_work(void *arg)
{
    SDL_LockMutex(s_lock);
    printf("                <============thread_work sleep\n");
    sleep(10);      // 用来测试获取锁
    printf("                <============thread_work wait\n");
    // 释放s_lock资源，并等待signal。之所以释放s_lock是让别的线程能够获取到s_lock
    SDL_CondWait(s_cond, s_lock); //另一个线程(1)发送signal和(2)释放lock后，这个函数退出

    printf("                <===========thread_work receive signal, continue to do ~_~!!!\n");
    printf("                <===========thread_work end\n");
    SDL_UnlockMutex(s_lock);
    return 0;
}

#undef main
int main()
{
    s_lock = SDL_CreateMutex();
    s_cond = SDL_CreateCond();
    SDL_Thread * t = SDL_CreateThread(thread_work,"thread_work",NULL);
    if(!t)
    {
        printf("  %s",SDL_GetError);
        return -1;
    }

    for(int i = 0;i< 2;i++)
    {
        sleep(2);
        printf("main execute =====>\n");
    }
    printf("main SDL_LockMutex(s_lock) before ====================>\n");
    SDL_LockMutex(s_lock);  // 获取锁，但是子线程还拿着锁
    printf("main ready send signal====================>\n");
    printf("main SDL_CondSignal(s_cond) before ====================>\n");
    SDL_CondSignal(s_cond); // 发送信号，唤醒等待的线程
    printf("main SDL_CondSignal(s_cond) after ====================>\n");
    sleep(10);
    SDL_UnlockMutex(s_lock);// 释放锁，让其他线程可以拿到锁
    printf("main SDL_UnlockMutex(s_lock) after ====================>\n");

    SDL_WaitThread(t, NULL);
    SDL_DestroyMutex(s_lock);
    SDL_DestroyCond(s_cond);

    return 0;
}
