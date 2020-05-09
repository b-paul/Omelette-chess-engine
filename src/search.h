#pragma once

#include "types.h"

struct PrincipalVariation {
    Move pv[2048];
    int length;
    int score;
    int depth;
};

struct Search {
    int infinite;
    int ponder;
    int maxDepth;
    int maxNodes;
    int stopped;
    int nodes;
    int time;
    int startTime;
};

Bitboard perft(Pos *board, int depth, int isRoot);
void initSearch();
Move getBestMove(Pos board, Thread *threads);
