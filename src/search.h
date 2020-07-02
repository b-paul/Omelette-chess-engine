#pragma once

#include "position.h"
#include "types.h"
#include "fathom/tbprobe.h"

struct PrincipalVariation {
    Move pv[256];
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

static inline unsigned probeSyzygyWDL(Pos *board) {
    // We only need to probe syzygy if we become an x man position
    // e.g. a position with 6 pieces and capture to 5 piece
    if (board->enPas ||
        board->castlePerms ||
        board->fiftyMoveRule ||
        popcnt(board->sides[WHITE] | board->sides[BLACK]) > (int)TB_LARGEST)
        return TB_RESULT_FAILED;

    return tb_probe_wdl(
            board->sides[WHITE], board->sides[BLACK],
            board->pieces[KING], board->pieces[QUEEN],
            board->pieces[ROOK], board->pieces[BISHOP],
            board->pieces[KNIGHT], board->pieces[PAWN],
            0, 0, 0, board->turn);
}
