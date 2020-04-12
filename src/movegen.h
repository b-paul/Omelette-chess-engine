#pragma once

#include "types.h"

struct Move {
    int value;
    int score;
};

struct MoveList {
    Move moves[2048];
    int count;
};

void genMoves(MoveList* moves, Pos *board, int type);

