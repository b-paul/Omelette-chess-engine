#ifdef TUNE

#define ENTRY_CNT 725000

#define PATH_TO_FENS "../tune/zurichess_positions/quiet-labeled.epd"

#include "types.h"

struct TuneEntry {
    float result;
    int eval;
};

void runTexelTuning(int threadCnt);

#endif
