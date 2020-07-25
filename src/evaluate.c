#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

#ifdef TUNE
EvalTrace T, emptyTrace;
#endif

int pawnValue = S(144, 231);

int knightValue = S(474, 418);

int bishopValue = S(519, 585);

int rookValue = S(694, 942);

int queenValue = S(1274, 1633);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-3, -3), S(7, -4), S(-1, -7), S(-26, 7), },
    {S(-9, -5), S(5, -5), S(5, -11), S(-3, -7), },
    {S(-20, 3), S(0, 0), S(11, -9), S(41, -15), },
    {S(-19, 9), S(1, 5), S(14, -4), S(48, -11), },
    {S(0, 35), S(20, 33), S(21, 27), S(39, 29), },
    {S(36, 55), S(65, 52), S(57, 52), S(42, 64), },
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },

};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-23, -14), S(-11, -19), S(-17, -13), S(-14, -12), },
    {S(-19, -17), S(-7, -4), S(1, -2), S(17, 9), },
    {S(-19, -12), S(3, 1), S(50, 18), S(41, 32), },
    {S(-8, -7), S(12, 6), S(38, 46), S(58, 56), },
    {S(-10, -7), S(13, 8), S(42, 46), S(67, 58), },
    {S(-15, -13), S(7, 4), S(40, 19), S(36, 21), },
    {S(-13, -14), S(-8, -4), S(-6, -47), S(-1, 18), },
    {S(-15, -17), S(-10, -25), S(-19, -13), S(-16, -11), },

};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-25, -13), S(-17, -10), S(-11, -19), S(-22, -11), },
    {S(-24, -13), S(44, -7), S(1, -1), S(21, 4), },
    {S(-12, -7), S(8, 1), S(27, 14), S(37, 19), },
    {S(-14, -6), S(10, 3), S(39, 18), S(51, 27), },
    {S(-19, -4), S(31, 7), S(7, 20), S(48, 28), },
    {S(-16, -3), S(7, 1), S(24, 15), S(26, 17), },
    {S(-23, -11), S(14, -6), S(2, 0), S(0, 10), },
    {S(-23, -16), S(-19, -9), S(-23, -19), S(-29, -7), },

};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-21, -15), S(-25, 0), S(27, -6), S(42, 0), },
    {S(-27, 2), S(-9, 2), S(3, 2), S(18, 4), },
    {S(-13, -6), S(-2, 0), S(4, 0), S(13, 3), },
    {S(-8, -8), S(-5, -3), S(3, 3), S(14, 3), },
    {S(-5, -4), S(-1, -1), S(8, 5), S(20, 7), },
    {S(4, 10), S(13, 14), S(10, 11), S(10, 16), },
    {S(2, 38), S(23, 39), S(24, 40), S(23, 38), },
    {S(11, 22), S(12, 32), S(34, 30), S(47, 27), },

};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(19, -19), S(15, -20), S(14, -19), S(130, -23), },
    {S(4, -35), S(8, -10), S(37, -5), S(30, 0), },
    {S(13, -28), S(10, -9), S(31, 23), S(29, 29), },
    {S(7, -25), S(15, 0), S(25, 26), S(24, 53), },
    {S(8, -27), S(16, 0), S(27, 27), S(23, 59), },
    {S(5, -30), S(17, -4), S(14, 24), S(17, 30), },
    {S(-10, -37), S(0, -3), S(24, -5), S(16, 2), },
    {S(-5, -18), S(-8, -20), S(-8, -19), S(29, -20), },

};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(62, -79), S(82, -72), S(21, -60), S(45, -73), },
    {S(2, -60), S(4, -31), S(-16, -23), S(-42, -19), },
    {S(-21, -50), S(-20, -20), S(-22, 2), S(-26, 3), },
    {S(-35, -53), S(-30, -22), S(-26, 9), S(-31, 30), },
    {S(-42, -55), S(-37, -20), S(-39, 7), S(-41, 28), },
    {S(-55, -53), S(-50, -23), S(-53, 3), S(-55, 5), },
    {S(-51, -60), S(-55, -28), S(-68, -23), S(-93, -18), },
    {S(-61, -79), S(-40, -69), S(-85, -59), S(-56, -72), },

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
