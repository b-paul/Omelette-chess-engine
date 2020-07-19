#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "types.h"

#ifdef TUNE
EvalTrace T, emptyTrace;
#endif

int pawnValue = S(2116, 19141);

int knightValue = S(12698, 14386);

int bishopValue = S(14634, 19471);

int rookValue = S(15537, 42727);

int queenValue = S(24412, 32331);

int pawnPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(171, 1460), S(928, 1066), S(-23, 761), S(-1155, 454), },
    {S(335, 661), S(367, 1049), S(226, 596), S(-905, 926), },
    {S(-781, 954), S(339, 1315), S(34, 618), S(2287, 611), },
    {S(-805, 623), S(419, 1237), S(-220, 256), S(2919, 533), },
    {S(-333, 647), S(156, 925), S(-465, 17), S(-591, 807), },
    {S(-218, 1189), S(1055, 772), S(-5, 504), S(-1501, 448), },
    {S(0, 0), S(0, 0), S(0, 0), S(0, 0), },

};

int knightPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(31, 10), S(-1384, -303), S(-124, 6), S(-140, -14), },
    {S(105, -36), S(77, 192), S(305, 307), S(640, 635), },
    {S(-441, 21), S(292, 438), S(3444, 1261), S(918, 1430), },
    {S(210, 330), S(605, 653), S(1288, 1923), S(2889, 2561), },
    {S(133, 316), S(780, 758), S(1175, 2029), S(2895, 2623), },
    {S(-539, 0), S(378, 631), S(3657, 1453), S(894, 1402), },
    {S(158, 55), S(71, 219), S(131, 189), S(506, 668), },
    {S(21, -13), S(-1418, -400), S(-169, -26), S(-134, 115), },

};

int bishopPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-9, 77), S(38, 83), S(-810, -300), S(-37, 128), },
    {S(-96, -26), S(1695, 799), S(554, 810), S(1260, 1119), },
    {S(310, 316), S(579, 878), S(1345, 1762), S(2527, 2401), },
    {S(387, 558), S(398, 881), S(1528, 1854), S(1678, 2542), },
    {S(237, 537), S(495, 1165), S(1416, 2030), S(1792, 2719), },
    {S(221, 432), S(502, 898), S(1257, 1861), S(2072, 2169), },
    {S(-34, 101), S(1579, 913), S(549, 859), S(1279, 1057), },
    {S(62, 38), S(31, 96), S(-1220, -374), S(-110, 257), },

};

int rookPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(439, 1299), S(-162, 1342), S(2698, 2098), S(2162, 2967), },
    {S(-60, 1941), S(757, 2357), S(1085, 2555), S(917, 2375), },
    {S(158, 1586), S(570, 1821), S(733, 2145), S(792, 2172), },
    {S(621, 1796), S(444, 1751), S(727, 2330), S(897, 2425), },
    {S(599, 1867), S(409, 1663), S(793, 2348), S(983, 2610), },
    {S(233, 1685), S(574, 2072), S(628, 2098), S(867, 2521), },
    {S(54, 2228), S(841, 2605), S(1153, 3050), S(851, 2639), },
    {S(74, 1642), S(-660, 1334), S(2435, 2206), S(2042, 3049), },

};

int queenPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(1167, 1995), S(653, 1674), S(627, 1825), S(4178, 2694), },
    {S(284, 612), S(652, 1035), S(2165, 1815), S(1613, 1931), },
    {S(580, 829), S(1300, 1251), S(1597, 1908), S(1601, 2083), },
    {S(545, 1134), S(923, 1240), S(1332, 1830), S(1901, 2432), },
    {S(570, 1097), S(1026, 1272), S(1318, 1833), S(1842, 2467), },
    {S(557, 852), S(1124, 1370), S(1207, 1813), S(1348, 2063), },
    {S(194, 621), S(748, 1285), S(1955, 1859), S(1278, 2036), },
    {S(1214, 2239), S(590, 1806), S(437, 1860), S(4749, 2993), },

};

int kingPSQT[RANK_CNT][FILE_CNT/2] = {
    {S(-1094, -2291), S(2492, -1642), S(-1335, -2171), S(-958, -2070), },
    {S(79, -962), S(794, 706), S(527, 830), S(-1022, -127), },
    {S(-114, -503), S(381, 1128), S(374, 1635), S(225, 1346), },
    {S(-88, -105), S(189, 973), S(434, 2227), S(364, 2425), },
    {S(-82, -152), S(211, 1114), S(316, 1953), S(330, 2234), },
    {S(-268, -629), S(71, 495), S(242, 1289), S(170, 1278), },
    {S(-99, -1341), S(257, 203), S(386, 544), S(-1123, -405), },
    {S(-1460, -2482), S(2011, -1385), S(-1828, -2378), S(-1378, -2530), },

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
