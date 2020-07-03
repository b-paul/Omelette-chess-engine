#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "search.h"
#include "threads.h"
#include "types.h"

#include <string.h>

Thread *initThreads(int threadCount, tTable *tt, HistoryTable *hTable) {
    Thread *threads = malloc(sizeof(Thread) * threadCount);

    for (int i = 0; i < threadCount; i++) {
        threads[i].index = i;
        threads[i].threads = threads;
        threads[i].threadCount = threadCount;

        threads[i].tt = tt;
        threads[i].hTable = hTable;
    }

    return threads;
}

void initThreadSearch(Thread *threads, Pos board, Search info) {
    for (int i = 0; i < threads[0].threadCount; i++) {
        threads[i].nodes = 0;
        threads[i].tbHits = 0;
        threads[i].seldepth = 0;
        threads[i].board = board;

        threads[i].startTime = info.startTime;
        threads[i].maxTime = info.time;
        threads[i].maxDepth = info.maxDepth;
        
        threads[i].infinite = info.infinite;
    }
}
