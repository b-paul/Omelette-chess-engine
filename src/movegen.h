#pragma once

#include "types.h"

struct Move {
    int value;
    int score;

    // Stored when making a capture for one turn
    // Used to undo captures
    int lastCapture;

};

struct MoveList {
    Move moves[2048];
    int count;
};

void genMoves(MoveList* moves, Pos *board, int type);

