#pragma once

#include <sys/time.h>

static inline int getTime() {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;;
}

static inline int timeLeft(Thread thread) {
    if (thread.infinite) return 1;
    int curTime = getTime();
    return thread.maxTime - (curTime - thread.startTime);
}
