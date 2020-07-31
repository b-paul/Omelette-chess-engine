#pragma once

#include "types.h"

#include <stdint.h>

#define S(mg, eg) ((int)((unsigned int)(eg) << 16) + mg)
#define mgS(s) ((int16_t)((uint16_t)((unsigned)(s))))
#define egS(s) ((int16_t)((uint16_t)(((unsigned)(s) + 0x8000)>>16)))

void initEval();
int phase(Pos *board);
int evaluate(Pos *board);

extern int PSQT[PIECE_CNT][SQ_CNT];

static inline int relativeFile(int sq) {
    return (sq & 0x4) ? (~sq & 0x3) : (sq & 0x3);
}

struct EvalTrace {
    int pawnValue[CL_CNT];
    int knightValue[CL_CNT];
    int bishopValue[CL_CNT];
    int rookValue[CL_CNT];
    int queenValue[CL_CNT];

    int pawnPSQT[RANK_CNT][FILE_CNT/2][CL_CNT];
    int knightPSQT[RANK_CNT][FILE_CNT/2][CL_CNT];
    int bishopPSQT[RANK_CNT][FILE_CNT/2][CL_CNT];
    int rookPSQT[RANK_CNT][FILE_CNT/2][CL_CNT];
    int queenPSQT[RANK_CNT][FILE_CNT/2][CL_CNT];
    int kingPSQT[RANK_CNT][FILE_CNT/2][CL_CNT];

    int knightMobilityBonus[9][CL_CNT];
    int bishopMobilityBonus[14][CL_CNT];
    int rookMobilityBonus[15][CL_CNT];
    int queenMobilityBonus[28][CL_CNT];
};

#ifdef TUNE
#define TRACE (1)
#else
#define TRACE (0)
#endif
