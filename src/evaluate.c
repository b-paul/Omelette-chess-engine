#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

#ifdef TUNE
EvalTrace T, emptyTrace;
#endif

int pawnValue = S(0, 0);
int knightValue = S(0, 0);
int bishopValue = S(0, 0);
int rookValue = S(0, 0);
int queenValue = S(0, 0);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int PSQT[PIECE_CNT][SQ_CNT];

int bishopPairBonus = S(20, 50);

void initEval() {
    // Add the material bonus to
    // each psqt square
    for (int i = 0; i < SQ_CNT; i++) {
        int r = rank(i);
        int f = relativeFile(i);

        PSQT[wP][i] = pawnValue + pawnPSQT[r][f];
        PSQT[wN][i] = knightValue + knightPSQT[r][f];
        PSQT[wB][i] = bishopValue + bishopPSQT[r][f];
        PSQT[wR][i] = rookValue + rookPSQT[r][f];
        PSQT[wQ][i] = queenValue + queenPSQT[r][f];
        PSQT[wK][i] = kingPSQT[r][f];

        PSQT[bP][i] = -pawnValue - pawnPSQT[7-r][f];
        PSQT[bN][i] = -knightValue - knightPSQT[7-r][f];
        PSQT[bB][i] = -bishopValue - bishopPSQT[7-r][f];
        PSQT[bR][i] = -rookValue - rookPSQT[7-r][f];
        PSQT[bQ][i] = -queenValue - queenPSQT[7-r][f];
        PSQT[bK][i] = -kingPSQT[7-r][f];
    }
}

#ifdef TUNE
void psqtEvalTrace(Pos *board) {
    int sq, r, f;
    Bitboard pieces = board->pieces[PAWN] & board->sides[WHITE];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.pawnValue[WHITE]++;
        T.pawnPSQT[r][f][WHITE]++;
    }
    pieces = board->pieces[PAWN] & board->sides[BLACK];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.pawnValue[BLACK]++;
        T.pawnPSQT[r][f][BLACK]++;
    }
    pieces = board->pieces[KNIGHT] & board->sides[WHITE];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.knightValue[WHITE]++;
        T.knightPSQT[r][f][WHITE]++;
    }
    pieces = board->pieces[KNIGHT] & board->sides[BLACK];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.knightValue[BLACK]++;
        T.knightPSQT[r][f][BLACK]++;
    }
    pieces = board->pieces[BISHOP] & board->sides[WHITE];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.bishopValue[WHITE]++;
        T.bishopPSQT[r][f][WHITE]++;
    }
    pieces = board->pieces[BISHOP] & board->sides[BLACK];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.bishopValue[BLACK]++;
        T.bishopPSQT[r][f][BLACK]++;
    }
    pieces = board->pieces[ROOK] & board->sides[WHITE];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.rookValue[WHITE]++;
        T.rookPSQT[r][f][WHITE]++;
    }
    pieces = board->pieces[ROOK] & board->sides[BLACK];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.rookValue[BLACK]++;
        T.rookPSQT[r][f][BLACK]++;
    }
    pieces = board->pieces[QUEEN] & board->sides[WHITE];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.queenValue[WHITE]++;
        T.queenPSQT[r][f][WHITE]++;
    }
    pieces = board->pieces[QUEEN] & board->sides[BLACK];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.queenValue[BLACK]++;
        T.queenPSQT[r][f][BLACK]++;
    }
    pieces = board->pieces[KING] & board->sides[WHITE];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.kingPSQT[r][f][WHITE]++;
    }
    pieces = board->pieces[KING] & board->sides[BLACK];
    while (pieces) {
        sq = poplsb(&pieces);
        r = rank(sq);
        f = relativeFile(sq);
        T.kingPSQT[r][f][BLACK]++;
    }
}
#endif

int phase(Pos *board) {
    int p = 24;
    p -= popcnt(board->pieces[KNIGHT] | board->pieces[BISHOP]);
    p -= popcnt(board->pieces[ROOK]) * 2;
    p -= popcnt(board->pieces[QUEEN]) * 4;
    return (p * 256 + 12)/24;
}

// Evaluate a board position
int evaluate(Pos *board) {
    
    // Return value

    int result = 0;

#ifdef TUNE
    psqtEvalTrace(board);
#endif

    result += board->psqtScore;

    int p = phase(board);
    int mg = mgS(result);
    int eg = egS(result);
    result = ((mg * (256-p)) + (eg * p))/256;

    return (board->turn ? -result : result);
}
