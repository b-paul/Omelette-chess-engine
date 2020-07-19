#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

#ifdef TUNE
EvalTrace T, emptyTrace;
#endif

int pawnValue = S(100, 150);
int knightValue = S(300, 250);
int bishopValue = S(325, 400);
int rookValue = S(500, 600);
int queenValue = S(900, 1100);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(10,-10), S(-5,-10), S(-8,-10), S(-20,-10)},
        {S(0,-10), S(-2,-10), S(2,-10), S(-5,-10)},
        {S(-5,-6), S(-5,-6), S(10,-6), S(25,-6)},
        {S(-5,0), S(-5,0), S(13,0), S(30,0)},
        {S(12,30), S(12,30), S(18,30), S(35,30)},
        {S(50,50), S(50,50), S(50,50), S(50,50)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(-20,-10), S(-10,-10), S(-10,-10), S(-10,-10)},
        {S(-15,-10), S(-5,-5), S(-5,-5), S(10,0)},
        {S(-15,-10), S(-5,-5), S(30,5), S(25,10)},
        {S(-10,-10), S(-3,-4), S(20,20), S(30,30)},
        {S(-10,-10), S(-5,-4), S(25,20), S(40,30)},
        {S(-10,-10), S(-5,-5), S(20,5), S(20,0)},
        {S(-10,-10), S(-5,-5), S(-10,-50), S(-10,10)},
        {S(-10,-10), S(-10,-10), S(-10,-10), S(-10,-10)},
};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(-20,-10), S(-18,-10), S(-10,-10), S(-18,-10)},
        {S(-20,-10), S(25,-8), S(-8,-8), S(15,-8)},
        {S(-20,-10), S(-5,-8), S(10,0), S(20,0)},
        {S(-20,-10), S(2,-8), S(25,0), S(30,8)},
        {S(-20,-10), S(25,-8), S(-4,0), S(23,8)},
        {S(-20,-10), S(-5,-8), S(8,0), S(10,0)},
        {S(-20,-10), S(-5,-8), S(-5,-8), S(-5,0)},
        {S(-20,-10), S(-20,-10), S(-20,-10), S(-20,-10)},
};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(-30,-20), S(-30,-18), S(5,-17), S(15,-15)},
        {S(-15,-18), S(-20,-16), S(-10,-15), S(5,-13)},
        {S(-10,-16), S(-9,-14), S(-5,-13), S(3,-11)},
        {S(-10,-14), S(-9,-12), S(-5,-11), S(3,-9)},
        {S(-5,-12), S(-4,-10), S(0,-9), S(8,-7)},
        {S(8,0), S(6,0), S(4,0), S(1,0)},
        {S(15,20), S(15,20), S(15,20), S(15,20)},
        {S(8,16), S(12,16), S(15,16), S(18,16)},
};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(4,-50), S(4,-50), S(4,-50), S(0,-50)},
        {S(-5,-50), S(-1,-30), S(2,-30), S(5,-30)},
        {S(-8,-50), S(-20,-30), S(4,-10), S(5,-10)},
        {S(-13,-50), S(-5,-30), S(2,-10), S(3,10)},
        {S(-13,-50), S(-6,-30), S(1,-10), S(2,10)},
        {S(-15,-50), S(-9,-30), S(-7,-10), S(-5,-10)},
        {S(-16,-50), S(-10,-30), S(-8,-30), S(-6,-30)},
        {S(-18,-50), S(-15,-50), S(-11,-50), S(-5,-50)},
};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
        {S(55,-50), S(50,-50), S(30,-50), S(30,-50)},
        {S(-10,-50), S(-10,-30), S(-10,-30), S(-10,-30)},
        {S(-20,-50), S(-20,-30), S(-20,-10), S(-20,-10)},
        {S(-30,-50), S(-30,-30), S(-30,-10), S(-30,10)},
        {S(-40,-50), S(-40,-30), S(-40,-10), S(-40,10)},
        {S(-50,-50), S(-50,-30), S(-50,-10), S(-50,-10)},
        {S(-60,-50), S(-60,-30), S(-60,-30), S(-60,-30)},
        {S(-70,-50), S(-70,-50), S(-70,-50), S(-70,-50)},
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
        PSQT[bK][i] = kingPSQT[7-r][f];
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
    return (p * 256 + 12)/12;
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
