#pragma once

#include "movegen.h"
#include "position.h"
#include "types.h"

struct MovePicker {
    int stage;

    Move ttMove;

    MoveList noisy;
    int noisyLeft;
    int noisyIndex;


    MoveList quiet;
    int quietLeft;
    int quietIndex;
};

enum MoveStages {
    TABLES_STAGE,
    GEN_NOISY,
    PICK_WIN_NOISY,
    PICK_KILLERS,
    GEN_QUIETS,
    PICK_QUIETS,
    PICK_LOSE_NOISY,
};

void initMovePicker(MovePicker *mp, Move ttMove);
Move selectNextMove(MovePicker *mp, Pos *board, int onlyNoisy);
