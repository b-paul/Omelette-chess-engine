#pragma once

#include "types.h"

struct goArgs {
    Pos board;
    char str[2048];
    Thread *threads;
};

void parseFen(const char *fen, Pos* board);
void bench(int argc, char **argv);
void uciLoop();
void reportSearchInfo(Thread *threads);
void reportMoveInfo(Move move, Pos board, const int index);
