#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

EvalTrace T, emptyTrace;

int pawnValue = S(101, 143);

int knightValue = S(358, 268);

int bishopValue = S(374, 287);

int rookValue = S(510, 522);

int queenValue = S(1017, 986);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-39, -39), S(10, -39), S(-3, -39), S(-11, -34), },
    {S(-35, -44), S(0, -43), S(-5, -53), S(-2, -47), },
    {S(-43, -35), S(-9, -39), S(-11, -51), S(7, -56), },
    {S(-41, -13), S(-2, -22), S(-10, -33), S(8, -46), },
    {S(-38, 57), S(-9, 59), S(21, 28), S(27, 20), },
    {S(14, 160), S(59, 143), S(55, 115), S(62, 104), },
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },

};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-41, -28), S(-10, -32), S(-27, -1), S(-13, 1), },
    {S(-13, -24), S(-26, -4), S(6, -1), S(17, 4), },
    {S(-10, -6), S(13, 1), S(22, 8), S(19, 25), },
    {S(-5, -2), S(17, 8), S(18, 31), S(20, 36), },
    {S(16, -4), S(16, 17), S(27, 35), S(33, 39), },
    {S(-7, -20), S(62, -10), S(34, 28), S(57, 21), },
    {S(-48, -18), S(-22, 0), S(61, -16), S(8, 9), },
    {S(-97, -62), S(-25, -41), S(-42, -11), S(0, -16), },

};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-20, 0), S(-2, 6), S(-2, 2), S(-5, 9), },
    {S(9, -9), S(31, -14), S(19, -3), S(14, 4), },
    {S(5, -6), S(12, -1), S(24, 3), S(14, 12), },
    {S(-2, -4), S(7, 0), S(5, 8), S(25, 9), },
    {S(-3, 1), S(3, 4), S(14, 8), S(33, 8), },
    {S(-14, 7), S(27, -5), S(27, 0), S(16, -3), },
    {S(-44, 1), S(1, -3), S(-4, 0), S(-11, -6), },
    {S(-18, -13), S(-11, -12), S(-46, -9), S(-28, -6), },

};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-9, -12), S(-12, 2), S(9, -2), S(26, -8), },
    {S(-45, 1), S(-5, -6), S(-2, -3), S(6, -4), },
    {S(-36, -3), S(-13, 0), S(-5, -7), S(-4, -4), },
    {S(-42, 4), S(-11, 1), S(-14, 4), S(-1, 0), },
    {S(-36, 9), S(-14, 2), S(5, 7), S(7, 0), },
    {S(-20, 5), S(26, 0), S(16, 0), S(11, 3), },
    {S(10, 5), S(13, 7), S(57, -4), S(57, -6), },
    {S(4, 7), S(14, 5), S(-8, 14), S(29, 6), },

};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(5, -29), S(10, -33), S(12, -27), S(30, -38), },
    {S(-7, -20), S(9, -30), S(31, -38), S(25, -22), },
    {S(0, -1), S(16, -21), S(4, 12), S(9, 0), },
    {S(2, -6), S(-7, 31), S(-4, 21), S(-10, 39), },
    {S(-1, 12), S(-21, 38), S(-10, 24), S(-21, 51), },
    {S(24, -16), S(18, -8), S(4, 16), S(4, 42), },
    {S(-2, -14), S(-45, 24), S(9, 19), S(-16, 39), },
    {S(5, -11), S(6, 6), S(15, 15), S(24, 15), },

};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(49, -69), S(64, -45), S(4, -21), S(28, -35), },
    {S(45, -33), S(30, -15), S(-19, 6), S(-40, 14), },
    {S(-13, -13), S(-4, 1), S(-41, 18), S(-60, 28), },
    {S(-42, -14), S(-18, 3), S(-40, 24), S(-64, 33), },
    {S(-12, -6), S(4, 20), S(2, 26), S(-7, 24), },
    {S(10, 4), S(35, 27), S(35, 27), S(15, 11), },
    {S(2, 0), S(8, 16), S(11, 22), S(15, 10), },
    {S(-5, -32), S(7, -13), S(4, -3), S(-2, -19), },

};

int knightMobilityBonus[9] = {
    S(-10, -11), S(-4, -11), S(-4, -1), S(-2, 3), S(4, 2), S(8, 6), S(13, 2), S(22, -4),
    S(37, -19),
};

int bishopMobilityBonus[14] = {
    S(-31, -57), S(-17, -32), S(-10, -21), S(-6, -8), S(-3, 2), S(2, 6), S(6, 10), S(1, 14),
    S(14, 13), S(18, 12), S(36, 8), S(39, 8), S(12, 13), S(15, 14),
};

int rookMobilityBonus[15] = {
    S(-39, -41), S(-37, -25), S(-35, -16), S(-28, -7), S(-23, 0), S(-15, 2), S(-7, 9), S(3, 9),
    S(10, 11), S(18, 11), S(28, 12), S(23, 14), S(31, 15), S(33, 15), S(50, 11),
};

int queenMobilityBonus[28] = {
    S(-44, -25), S(-39, -17), S(-25, -45), S(-21, -55), S(-19, -47), S(-14, -50), S(-14, -31), S(-13, -15),
    S(-8, -8), S(-6, -4), S(-3, 3), S(1, 11), S(-1, 15), S(4, 17), S(6, 29), S(19, 27),
    S(21, 17), S(32, 18), S(39, 19), S(45, 26), S(30, 31), S(34, 34), S(28, 31), S(32, 43),
    S(16, 26), S(16, 25), S(5, 9), S(3, 5),
};

int bishopPairBonus = S(23, 50);

int TempoBonus = 20;

int PSQT[PIECE_CNT][SQ_CNT];

void initEval() {
    // Add the material bonus to
    // each psqt square
    int r, f;
    for (int i = 0; i < SQ_CNT; i++) {
        r = rank(i);
        f = relativeFile(i);

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

int evaluateKnights(Pos *board, Bitboard knights, const Bitboard moveableSquares, const int turn) {
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

int evaluateBishops(Pos *board, Bitboard bishops, const Bitboard moveableSquares, const Bitboard occ, const int turn) {
    int r = 0;
    int sq, mobility;

    Bitboard attacks;

    if (bishops & LightSquares && bishops & DarkSquares) {
        r += bishopPairBonus;
        if (TRACE) T.bishopPairBonus[turn]++;
    }

    while(bishops) {
        sq = poplsb(&bishops);

        if (TRACE) T.bishopValue[turn]++;
        if (TRACE) T.bishopPSQT[relativeRank(sq, turn)][relativeFile(sq)][turn]++;

        // Track how many bishops we have for bishop pair bonus

        attacks = getBishopAttacks(sq, occ) & moveableSquares;
        mobility = popcnt(attacks);

        r += bishopMobilityBonus[mobility];
        if (TRACE) T.bishopMobilityBonus[mobility][turn]++;
    }

    return r;
}

int evaluateRooks(Pos *board, Bitboard rooks, const Bitboard moveableSquares, const Bitboard occ, const int turn) {
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

int evaluateQueens(Pos *board, Bitboard queens, const Bitboard moveableSquares, const Bitboard occ, const int turn) {
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
    result = ((mg * (256-p)) + (eg * p))/256 + TempoBonus;

#ifdef TUNE
    result = TempoBonus;
#endif

    return (board->turn ? -result : result);
}
