#pragma once

#include "types.h"

struct goArgs {
    Pos board;
    char str[2048];
    Thread *threads;
};

void parseFen(const char *fen, Pos* board);
void uciLoop();
