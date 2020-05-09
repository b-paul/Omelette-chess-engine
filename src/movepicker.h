#pragma once

#include "movegen.h"
#include "position.h"
#include "types.h"

struct MovePicker {
    int stage;

    int height;

    Move ttMove;

    MoveList noisy;
    int noisyLeft;


    MoveList quiet;
    int quietLeft;

    Undo undo;
};

struct HistoryTable {
    Move killers[2048][2];

    // Turn, from, to
    Key historyScores[CL_CNT][SQ_CNT][SQ_CNT];
};

enum MoveStages {
    TABLES_STAGE,
    GEN_NOISY,
    PICK_WIN_NOISY,
    GEN_QUIETS,
    PICK_QUIETS,
    PICK_LOSE_NOISY,
};

void initHistoryTable(HistoryTable *table);
void updateHistoryScores(HistoryTable *table, Pos board, Move move, int depth, int height);
void initMovePicker(MovePicker *mp, Pos *board, Move ttMove, int height);
Move selectNextMove(MovePicker *mp, HistoryTable *table, Pos *board, int onlyNoisy);
