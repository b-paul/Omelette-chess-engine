#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

EvalTrace T, emptyTrace;

int pawnValue = S(101, 143);

int knightValue = S(357, 267);

int bishopValue = S(393, 289);

int rookValue = S(509, 521);

int queenValue = S(1005, 990);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-38, -39), S(10, -39), S(-3, -38), S(-11, -34), },
    {S(-35, -44), S(0, -43), S(-5, -53), S(-2, -47), },
    {S(-43, -35), S(-9, -39), S(-11, -51), S(7, -55), },
    {S(-40, -13), S(-2, -22), S(-10, -33), S(8, -46), },
    {S(-37, 57), S(-9, 59), S(21, 28), S(28, 20), },
    {S(14, 159), S(59, 143), S(55, 115), S(61, 104), },
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },

};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-39, -27), S(-11, -32), S(-27, 0), S(-13, 1), },
    {S(-14, -23), S(-25, -4), S(6, -1), S(17, 4), },
    {S(-11, -5), S(13, 0), S(22, 7), S(19, 24), },
    {S(-5, -1), S(17, 8), S(18, 31), S(20, 36), },
    {S(16, -3), S(15, 16), S(27, 35), S(33, 38), },
    {S(-8, -19), S(60, -11), S(34, 28), S(58, 20), },
    {S(-46, -19), S(-21, 0), S(60, -15), S(8, 9), },
    {S(-94, -63), S(-24, -40), S(-40, -11), S(0, -16), },

};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-19, 0), S(0, 6), S(-2, 4), S(-5, 10), },
    {S(10, -9), S(31, -13), S(20, -3), S(14, 5), },
    {S(5, -5), S(12, -1), S(24, 3), S(15, 11), },
    {S(-1, -3), S(8, 0), S(6, 8), S(25, 9), },
    {S(-3, 2), S(4, 4), S(15, 8), S(34, 7), },
    {S(-14, 6), S(26, -5), S(26, 0), S(17, -3), },
    {S(-43, 1), S(3, -3), S(-3, 0), S(-9, -6), },
    {S(-17, -13), S(-11, -13), S(-43, -10), S(-26, -7), },

};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-8, -12), S(-12, 1), S(9, -3), S(26, -9), },
    {S(-45, 1), S(-4, -7), S(-2, -3), S(6, -4), },
    {S(-35, -3), S(-13, 0), S(-5, -7), S(-4, -4), },
    {S(-41, 4), S(-11, 1), S(-14, 4), S(-1, 0), },
    {S(-36, 8), S(-14, 2), S(6, 7), S(7, 0), },
    {S(-19, 5), S(25, 0), S(16, 0), S(11, 3), },
    {S(10, 5), S(12, 7), S(56, -4), S(54, -5), },
    {S(3, 8), S(14, 4), S(-7, 14), S(28, 7), },

};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(3, -28), S(9, -32), S(11, -27), S(29, -38), },
    {S(-8, -19), S(8, -29), S(31, -38), S(25, -21), },
    {S(0, 0), S(16, -20), S(4, 13), S(8, 1), },
    {S(2, -6), S(-8, 30), S(-5, 22), S(-11, 40), },
    {S(-1, 12), S(-22, 37), S(-10, 23), S(-21, 51), },
    {S(23, -15), S(17, -7), S(5, 15), S(6, 41), },
    {S(-2, -13), S(-45, 22), S(10, 19), S(-14, 36), },
    {S(4, -10), S(6, 6), S(15, 15), S(24, 16), },

};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(48, -69), S(64, -45), S(3, -21), S(27, -35), },
    {S(44, -33), S(30, -15), S(-19, 6), S(-40, 14), },
    {S(-13, -13), S(-4, 1), S(-41, 18), S(-60, 28), },
    {S(-41, -14), S(-16, 3), S(-38, 24), S(-61, 32), },
    {S(-11, -6), S(4, 20), S(3, 26), S(-6, 24), },
    {S(9, 4), S(32, 27), S(32, 28), S(14, 11), },
    {S(1, 0), S(8, 16), S(11, 22), S(14, 10), },
    {S(-4, -30), S(6, -13), S(3, -3), S(-2, -18), },

};

int knightMobilityBonus[9] = {
    S(-10, -13), S(-4, -12), S(-4, -2), S(-2, 2), S(4, 2), S(8, 6), S(13, 2), S(22, -3),
    S(37, -18),
};

int bishopMobilityBonus[14] = {
    S(-29, -56), S(-14, -32), S(-7, -21), S(-3, -7), S(0, 2), S(4, 6), S(9, 10), S(4, 14),
    S(16, 13), S(20, 11), S(37, 8), S(38, 9), S(11, 14), S(14, 14),
};

int rookMobilityBonus[15] = {
    S(-39, -42), S(-37, -26), S(-35, -16), S(-28, -8), S(-23, 0), S(-15, 2), S(-7, 9), S(3, 9),
    S(10, 11), S(18, 11), S(28, 12), S(24, 14), S(30, 16), S(32, 15), S(48, 12),
};

int queenMobilityBonus[28] = {
    S(-44, -23), S(-39, -15), S(-25, -42), S(-21, -54), S(-19, -47), S(-14, -50), S(-14, -31), S(-13, -15),
    S(-8, -8), S(-6, -5), S(-3, 3), S(1, 11), S(-1, 14), S(4, 16), S(6, 29), S(19, 27),
    S(21, 17), S(31, 18), S(38, 20), S(43, 27), S(29, 31), S(33, 34), S(27, 31), S(31, 42),
    S(15, 25), S(15, 24), S(4, 8), S(3, 4),
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
        r = 7-rank(sq);
        f = relativeFile(sq);
        T.pawnValue[BLACK]++;
        T.pawnPSQT[r][f][BLACK]++;
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
        r = 7-rank(sq);
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

int evaluateKnights(Pos *board, Bitboard knights, Bitboard moveableSquares, int turn) {
    int r = 0;
    int sq, mobility;

    Bitboard attacks;

    while(knights) {
        sq = poplsb(&knights);

        if (TRACE) T.knightValue[turn]++;
        if (TRACE) T.knightPSQT[relativeRank(sq, turn)][relativeFile(sq)][turn]++;

        attacks = getKnightAttacks(sq) & moveableSquares;
        mobility = popcnt(attacks);

        r += knightMobilityBonus[mobility];
        if (TRACE) T.knightMobilityBonus[mobility][turn]++;
    }

    return r;
}

int evaluateBishops(Pos *board, Bitboard bishops, Bitboard moveableSquares, Bitboard occ, int turn) {
    int r = 0;
    int sq, mobility;

    Bitboard attacks;

    while(bishops) {
        sq = poplsb(&bishops);

        if (TRACE) T.bishopValue[turn]++;
        if (TRACE) T.bishopPSQT[relativeRank(sq, turn)][relativeFile(sq)][turn]++;

        attacks = getBishopAttacks(sq, occ) & moveableSquares;
        mobility = popcnt(attacks);

        r += bishopMobilityBonus[mobility];
        if (TRACE) T.bishopMobilityBonus[mobility][turn]++;
    }

    return r;
}

int evaluateRooks(Pos *board, Bitboard rooks, Bitboard moveableSquares, Bitboard occ, int turn) {
    int r = 0;
    int sq, mobility;

    Bitboard attacks;

    while(rooks) {
        sq = poplsb(&rooks);

        if (TRACE) T.rookValue[turn]++;
        if (TRACE) T.rookPSQT[relativeRank(sq, turn)][relativeFile(sq)][turn]++;

        attacks = getRookAttacks(sq, occ) & moveableSquares;
        mobility = popcnt(attacks);

        r += rookMobilityBonus[mobility];
        if (TRACE) T.rookMobilityBonus[mobility][turn]++;
    }

    return r;
}

int evaluateQueens(Pos *board, Bitboard queens, Bitboard moveableSquares, Bitboard occ, int turn) {
    int r = 0;
    int sq, mobility;

    Bitboard attacks;

    while(queens) {
        sq = poplsb(&queens);

        if (TRACE) T.queenValue[turn]++;
        if (TRACE) T.queenPSQT[relativeRank(sq, turn)][relativeFile(sq)][turn]++;

        attacks = getQueenAttacks(sq, occ) & moveableSquares;
        mobility = popcnt(attacks);

        r += queenMobilityBonus[mobility];
        if (TRACE) T.queenMobilityBonus[mobility][turn]++;
    }

    return r;
}

// Evaluate a board position
int evaluate(Pos *board) {
    
    // Return value

    int result = 0;

#ifdef TUNE
    psqtEvalTrace(board);
#endif

    Bitboard wKnights = board->pieces[KNIGHT] & board->sides[WHITE];
    Bitboard bKnights = board->pieces[KNIGHT] & board->sides[BLACK];
    Bitboard wBishops = board->pieces[BISHOP] & board->sides[WHITE];
    Bitboard bBishops = board->pieces[BISHOP] & board->sides[BLACK];
    Bitboard wRooks = board->pieces[ROOK] & board->sides[WHITE];
    Bitboard bRooks = board->pieces[ROOK] & board->sides[BLACK];
    Bitboard wQueens = board->pieces[QUEEN] & board->sides[WHITE];
    Bitboard bQueens = board->pieces[QUEEN] & board->sides[BLACK];

    Bitboard wMoveableSquares = ~(board->sides[WHITE] | soeaOne(board->pieces[PAWN]) | soweOne(board->pieces[PAWN]));
    Bitboard bMoveableSquares = ~(board->sides[BLACK] | noeaOne(board->pieces[PAWN]) | noweOne(board->pieces[PAWN]));
    Bitboard occ = board->sides[WHITE] | board->sides[BLACK];

    result += evaluateKnights(board, wKnights, wMoveableSquares, WHITE);
    result -= evaluateKnights(board, bKnights, bMoveableSquares, BLACK);
    result += evaluateBishops(board, wBishops, wMoveableSquares, occ, WHITE);
    result -= evaluateBishops(board, bBishops, bMoveableSquares, occ, BLACK);
    result += evaluateRooks(board, wRooks, wMoveableSquares, occ, WHITE);
    result -= evaluateRooks(board, bRooks, bMoveableSquares, occ, BLACK);
    result += evaluateQueens(board, wQueens, wMoveableSquares, occ, WHITE);
    result -= evaluateQueens(board, bQueens, bMoveableSquares, occ, BLACK);
    
    result += board->psqtScore;

    int p = phase(board);
    int mg = mgS(result);
    int eg = egS(result);
    result = ((mg * (256-p)) + (eg * p))/256;

    return (board->turn ? -result : result);
}
