#include "movegen.h"
#include "position.h"
#include "search.h"
#include "threads.h"
#include "types.h"

#include <string.h>

Thread *initThreads(int threadCount, tTable *tt) {
    Thread *threads = malloc(sizeof(Thread) * threadCount);

    for (int i = 0; i < threadCount; i++) {
        threads[i].index = i;
        threads[i].threads = threads;
        threads[i].threadCount = threadCount;

        threads[i].tt = tt;
    }

    return threads;
}

void initThreadSearch(Thread *threads, Pos board, Search info) {
    for (int i = 0; i < threads[0].threadCount; i++) {
        threads[i].nodes = 0;
        threads[i].board = board;

        threads[i].startTime = info.startTime;
        threads[i].maxTime = info.time;
        
        threads[i].infinite = info.infinite;
    }
}
