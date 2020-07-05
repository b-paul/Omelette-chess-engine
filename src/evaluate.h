#pragma once

#include "types.h"

#include <stdint.h>

#define S(mg, eg) ((int)((unsigned int)(eg) << 16) + mg)
#define mgS(mg) ((int16_t)((uint16_t)((unsigned)(mg) & 0xFFFF)))
#define egS(eg) ((int16_t)((uint16_t)(((unsigned)(eg) + 0x8000)>>16)))

void initEval();
int phase(Pos *board);
int evaluate(Pos *board);

extern int PSQT[PIECE_TYPE_CNT][SQ_CNT];
