#include "movegen.h"
#include "time.h"
#include "threads.h"
#include "types.h"

int initTimeManagement(Pos *board, int time, int movesLeft) {
    int moveCnt = legalMoveCount(board);

    // If there is only 1 legal move, or no moves
    // then there is no reason to search the position
    if (moveCnt <= 1) {
        return 0;
    }

    return time/movesLeft;
}

void updateTimeManagement(Thread *thread) {
    // We have found mate, so stop searching
    if (abs(thread->score) >= 997951) {
        thread->maxTime = 0;
        return;
    }
    // If there is not enough time to search
    // the next depth, stop
    // We assume that we cannot search the next
    // depth if we have used 50% of the search time
    if (timeSearched(thread) > (thread->maxTime / 2)) {
        thread->maxTime = 0;
        return;
    }
}
