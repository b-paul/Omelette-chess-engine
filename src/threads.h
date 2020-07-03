#pragma once

#include "evaluate.h"
#include "movepicker.h"
#include "search.h"
#include "tt.h"
#include "types.h"

#include <setjmp.h>
#include <stdbool.h>

struct Thread {
    int count;
    Move bestMove;
    int depth;
    int seldepth;
    Pos board;
    unsigned long long nodes;
    int tbHits;
    int startTime;
    int maxTime;
    int maxDepth;
    Score score;
    bool infinite;

    PrincipalVariation pv;

    jmp_buf jumpEnv;

    tTable *tt;

    HistoryTable *hTable;

    // For getting information about the entire search
    Thread *threads;

    int index;
    int threadCount;
};

Thread *initThreads(int threadCount, tTable *tt, HistoryTable *hTable);
void initThreadSearch(Thread *threads, Pos board, Search info);
