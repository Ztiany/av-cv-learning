#include "times.h"
#include <sys/time.h>

//当前时间戳 clock
long long getNowInMillisecond() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    int sec = tv.tv_sec % 360000;
    long long t = sec * 1000 + tv.tv_usec / 1000;
    return t;
}