#pragma once

#include "search.h"
#include "threads.h"
#include "types.h"

#include <sys/time.h>
#include <stddef.h>

int initTimeManagement(Pos *board, const int time, const int inc, const int movesLeft);
void updateTimeManagement(Thread *thread);

static inline int legalMoveCount(Pos *board) {
    return perft(board, 1);
}

static inline int getTime() {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;;
}

static inline int timeSearched(Thread *thread) {
    int curTime = getTime();
    return curTime - thread->startTime;
}

static inline int stopSearch(Thread *thread) {
    return thread->depth > 1 &&
           (thread->nodes & 2047) == 2047 &&
           !(thread->infinite) &&
           timeSearched(thread) > thread->maxTime;
}
