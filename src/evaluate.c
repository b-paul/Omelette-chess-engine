#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

EvalTrace T, emptyTrace;

int pawnValue = S(111, 147);

int knightValue = S(383, 278);

int bishopValue = S(401, 298);

int rookValue = S(549, 544);

int queenValue = S(1131, 1001);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-44, -41), S(9, -42), S(-6, -41), S(-14, -35), },
    {S(-38, -47), S(0, -45), S(-4, -56), S(-1, -49), },
    {S(-48, -38), S(-13, -41), S(-11, -55), S(7, -59), },
    {S(-44, -16), S(0, -25), S(-11, -36), S(11, -50), },
    {S(-38, 55), S(-8, 58), S(24, 25), S(28, 16), },
    {S(0, 185), S(81, 160), S(63, 130), S(84, 114), },
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },

};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-57, -26), S(-14, -33), S(-24, -3), S(-14, 2), },
    {S(-16, -25), S(-24, -1), S(9, 0), S(19, 6), },
    {S(-12, -5), S(17, 1), S(24, 10), S(25, 27), },
    {S(-4, -2), S(24, 11), S(26, 34), S(26, 38), },
    {S(22, -5), S(21, 18), S(38, 37), S(39, 40), },
    {S(-1, -21), S(75, -14), S(47, 27), S(73, 18), },
    {S(-43, -19), S(-14, 0), S(85, -22), S(17, 5), },
    {S(-145, -52), S(-49, -41), S(-79, -7), S(0, -19), },

};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-16, 0), S(1, 4), S(0, 2), S(1, 9), },
    {S(15, -12), S(38, -16), S(23, -2), S(19, 3), },
    {S(9, -7), S(17, -1), S(32, 5), S(18, 14), },
    {S(1, -6), S(12, -1), S(9, 10), S(29, 12), },
    {S(-1, 1), S(4, 6), S(19, 10), S(41, 11), },
    {S(-25, 10), S(30, -3), S(32, 1), S(21, -3), },
    {S(-40, 0), S(27, -7), S(8, 0), S(-7, -6), },
    {S(-29, -17), S(-29, -13), S(-93, -2), S(-65, 0), },

};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-12, -12), S(-15, 2), S(8, -3), S(27, -11), },
    {S(-51, 3), S(-6, -7), S(-3, -2), S(7, -5), },
    {S(-41, -1), S(-14, 0), S(-7, -7), S(-5, -4), },
    {S(-41, 7), S(-11, 2), S(-13, 5), S(0, 0), },
    {S(-36, 13), S(-17, 7), S(8, 9), S(9, 1), },
    {S(-17, 8), S(30, 0), S(19, 2), S(14, 3), },
    {S(14, 8), S(19, 8), S(70, -5), S(73, -10), },
    {S(-1, 11), S(20, 2), S(-12, 14), S(39, 2), },

};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(17, -44), S(13, -40), S(16, -31), S(31, -41), },
    {S(-3, -26), S(16, -38), S(35, -40), S(29, -23), },
    {S(5, -5), S(23, -26), S(8, 11), S(12, -1), },
    {S(5, -11), S(-8, 38), S(0, 21), S(-4, 39), },
    {S(-2, 19), S(-20, 43), S(-12, 33), S(-15, 52), },
    {S(26, -12), S(21, 1), S(15, 18), S(1, 54), },
    {S(8, -15), S(-24, 18), S(15, 27), S(-36, 62), },
    {S(13, -17), S(5, 9), S(18, 20), S(33, 12), },

};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(47, -74), S(64, -50), S(0, -22), S(23, -37), },
    {S(44, -36), S(29, -15), S(-24, 8), S(-49, 18), },
    {S(-17, -13), S(-4, 3), S(-44, 21), S(-66, 32), },
    {S(-64, -9), S(-31, 9), S(-56, 31), S(-86, 40), },
    {S(-29, -1), S(-3, 27), S(-5, 31), S(-22, 29), },
    {S(21, 3), S(67, 26), S(64, 26), S(38, 7), },
    {S(6, -2), S(20, 16), S(17, 24), S(34, 7), },
    {S(-8, -48), S(24, -20), S(11, -9), S(-1, -22), },

};

int knightMobilityBonus[9] = {
    S(-7, -6), S(0, -10), S(-1, 0), S(1, 4), S(7, 3), S(11, 7), S(16, 3), S(25, -4),
    S(39, -20),
};

int bishopMobilityBonus[14] = {
    S(-35, -52), S(-18, -28), S(-11, -16), S(-7, -3), S(-4, 6), S(1, 10), S(5, 13), S(1, 15),
    S(13, 14), S(18, 11), S(40, 5), S(50, 4), S(20, 6), S(32, 8),
};

int rookMobilityBonus[15] = {
    S(-40, -40), S(-37, -24), S(-34, -13), S(-26, -4), S(-20, 4), S(-12, 6), S(-5, 13), S(6, 13),
    S(11, 15), S(18, 14), S(29, 13), S(21, 16), S(30, 14), S(37, 11), S(72, 3),
};

int queenMobilityBonus[28] = {
    S(-45, -58), S(-39, -44), S(-24, -67), S(-21, -63), S(-20, -49), S(-15, -48), S(-15, -26), S(-14, -7),
    S(-10, 0), S(-7, 4), S(-4, 14), S(0, 20), S(-1, 24), S(4, 25), S(10, 35), S(22, 34),
    S(26, 20), S(35, 21), S(49, 18), S(60, 22), S(44, 27), S(53, 30), S(42, 23), S(46, 39),
    S(23, 33), S(27, 34), S(13, 24), S(9, 12),
};

int bishopPairBonus = S(25, 51);

int PSQT[PIECE_CNT][SQ_CNT];

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
