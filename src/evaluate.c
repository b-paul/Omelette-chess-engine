#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

#ifdef TUNE
EvalTrace T, emptyTrace;
#endif

int pawnValue = S(-1172, 3666);

int knightValue = S(-2167, 10149);

int bishopValue = S(-2715, 11093);

int rookValue = S(-9278, 14071);

int queenValue = S(-17237, 26314);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(205, 235), S(442, 853), S(70, 327), S(-906, 225), },
    {S(145, 210), S(-49, 337), S(56, 16), S(-524, -313), },
    {S(-544, -244), S(-155, 216), S(167, -80), S(1211, 795), },
    {S(-571, -266), S(-151, 230), S(47, -208), S(1426, 906), },
    {S(-240, -112), S(-237, 186), S(-193, -219), S(-387, -246), },
    {S(-91, -49), S(291, 653), S(-40, 231), S(-933, 66), },
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(328, -391), S(-290, -638), S(173, -279), S(64, -170), },
    {S(389, -398), S(-14, 18), S(-98, 216), S(-239, 529), },
    {S(-85, -209), S(-234, 346), S(911, 1522), S(-599, 981), },
    {S(-31, 104), S(-372, 638), S(-681, 1205), S(-232, 1624), },
    {S(-142, 139), S(-349, 717), S(-725, 1153), S(-271, 1647), },
    {S(-78, -283), S(-346, 461), S(805, 1509), S(-629, 962), },
    {S(257, -240), S(17, -26), S(-110, 82), S(-283, 414), },
    {S(395, -462), S(-209, -871), S(173, -294), S(45, -168), },
};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(176, -243), S(69, -83), S(139, -421), S(22, -78), },
    {S(134, -206), S(314, 565), S(-192, 354), S(-22, 683), },
    {S(10, 116), S(-252, 458), S(-368, 814), S(-46, 1131), },
    {S(-88, 205), S(-297, 408), S(-257, 867), S(-410, 954), },
    {S(-108, 151), S(-381, 521), S(-379, 862), S(-456, 1003), },
    {S(-124, 196), S(-277, 415), S(-378, 746), S(-177, 974), },
    {S(121, -171), S(148, 468), S(-213, 357), S(-35, 584), },
    {S(329, -355), S(54, -80), S(-142, -640), S(-65, -24), },
};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(68, 174), S(-522, 250), S(316, 937), S(-243, 814), },
    {S(-533, 280), S(-457, 540), S(-423, 584), S(-411, 546), },
    {S(-264, 178), S(-328, 388), S(-359, 448), S(-348, 453), },
    {S(-128, 202), S(-212, 241), S(-376, 462), S(-313, 456), },
    {S(-190, 232), S(-190, 209), S(-372, 474), S(-323, 457), },
    {S(-251, 203), S(-348, 413), S(-264, 336), S(-377, 478), },
    {S(-458, 290), S(-443, 572), S(-472, 645), S(-427, 526), },
    {S(-363, -158), S(-531, 97), S(72, 875), S(-291, 705), },
};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-304, 435), S(-436, 413), S(-536, 465), S(75, 1054), },
    {S(-186, 148), S(-305, 358), S(-440, 913), S(-775, 947), },
    {S(-395, 447), S(-421, 644), S(-827, 1152), S(-958, 1252), },
    {S(-807, 684), S(-673, 838), S(-824, 1071), S(-1016, 1433), },
    {S(-703, 576), S(-668, 845), S(-846, 1089), S(-1156, 1552), },
    {S(-374, 397), S(-654, 776), S(-867, 1013), S(-992, 1121), },
    {S(-170, 73), S(-413, 472), S(-474, 790), S(-872, 832), },
    {S(-370, 469), S(-424, 326), S(-456, 277), S(13, 1228), },
};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(837, -1047), S(1221, 406), S(258, -640), S(421, -769), },
    {S(273, -271), S(5, 60), S(-146, 148), S(-381, -103), },
    {S(59, -157), S(-279, 250), S(-366, 349), S(-406, 374), },
    {S(115, -240), S(-274, 232), S(-534, 552), S(-573, 601), },
    {S(118, -243), S(-326, 290), S(-497, 492), S(-560, 571), },
    {S(73, -222), S(-258, 183), S(-411, 371), S(-475, 420), },
    {S(224, -313), S(-107, -17), S(-185, 120), S(-453, -72), },
    {S(699, -1072), S(774, 239), S(100, -710), S(100, -826), },

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
