#pragma once

#include "search.h"
#include "threads.h"
#include "types.h"

#include <sys/time.h>
#include <stddef.h>

extern int initTimeManagement(Pos *board, int time, int movesLeft);

static inline int legalMoveCount(Pos *board) {
    return perft(board, 1, 0);
}

static inline int getTime() {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;;
}

static inline int timeSearched(Thread *thread) {
    if (thread->infinite) return 1;
    int curTime = getTime();
    return curTime - thread->startTime;
}

static inline int stopSearch(Thread *thread) {
    return thread->depth > 1 &&
           (thread->nodes & 2047) == 2047 &&
           !(thread->infinite) &&
           timeSearched(thread) > thread->maxTime;
}
