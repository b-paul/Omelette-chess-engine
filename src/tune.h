#ifdef TUNE

#define ENTRY_CNT 725000
#define K_PRECISION 10
// Mini-batch size
#define BATCH_SIZE 4096
#define PARAM_CNT 197

#define PATH_TO_FENS "../tune/zurichess_positions/quiet-labeled.epd"

#include "types.h"

struct TuneEntry {
    float result;
    int eval;
    int phase;

    int evalDiff[PARAM_CNT][PHASE_CNT];
};

void runTexelTuning(int threadCnt);

#endif
