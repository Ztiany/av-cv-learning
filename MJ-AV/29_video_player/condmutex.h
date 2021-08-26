#ifndef CONDMUTEX_H
#define CONDMUTEX_H
#include <SDL2/SDL.h>

/**互斥锁封装*/
class CondMutex {
public:
  CondMutex();
  ~CondMutex();
  void lock();
  void unlock();
  void signal();
  void broadcast();
  void wait();

private:
  SDL_mutex *_mutex = nullptr;
  SDL_cond *_cond = nullptr;
};

#endif // CONDMUTEX_H
