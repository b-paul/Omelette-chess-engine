#pragma once

#include "movepicker.h"
#include "search.h"
#include "tt.h"
#include "types.h"

#include <setjmp.h>

struct Thread {
    int count;
    Move bestMove;
    int depth;
    int seldepth;
    Pos board;
    int nodes;
    int startTime;
    int maxTime;
    int maxDepth;
    int score;
    int infinite;

    PrincipalVariation pv;

    // For getting information about the entire search
    Thread *threads;

    int index;
    int threadCount;

    jmp_buf jumpEnv;

    tTable *tt;

    HistoryTable *hTable;
};

Thread *initThreads(int threadCount, tTable *tt, HistoryTable *hTable);
void initThreadSearch(Thread *threads, Pos board, Search info);
