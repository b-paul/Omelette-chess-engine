#include "movegen.h"
#include "time.h"
#include "threads.h"
#include "types.h"

int initTimeManagement(Pos *board, int time, int movesLeft) {
    int moveCnt = min(legalMoveCount(board), 5);

    // If there is only 1 legal move, or no moves
    // then there is no reason to search the position
    if (moveCnt <= 1) {
        return 0;
    }

    return (time*moveCnt*moveCnt)/(movesLeft*25);
}

void updateTimeManagement(Thread *thread) {
}
