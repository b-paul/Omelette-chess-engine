#include "movegen.h"
#include "time.h"
#include "threads.h"
#include "types.h"

int initTimeManagement(Pos *board, const int time, const int inc, const int movesLeft) {
    int moveCnt = MIN(legalMoveCount(board), 5);

    // If there is only 1 legal move, or no moves
    // then there is no reason to search the position
    if (moveCnt <= 1) {
        return 0;
    }

    int allocatedTime = ((time+inc)*moveCnt*moveCnt)/(movesLeft*25);

    if (allocatedTime > time)
        allocatedTime = (time*moveCnt*moveCnt)/(movesLeft*25);

    return allocatedTime;
}

void updateTimeManagement(Thread *thread) {
    // If there is not enough time to search
    // the next depth, stop
    // We assume that we cannot search the next
    // depth if we have used 50% of the search time
    if (timeSearched(thread) > (thread->maxTime / 2)) {
        thread->maxTime = 0;
        return;
    }
}
