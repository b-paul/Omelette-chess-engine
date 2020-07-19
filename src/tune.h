#ifdef TUNE

#define ENTRY_CNT (725000)
#define PARTITION_CNT (64)
#define K_PRECISION (10)
// Mini-batch size
#define BATCH_SIZE (16384)

#define LR_DROP_RATE (1.25)
#define LEARNING_RATE (5.0)

#define PARAM_CNT (197)

#define STACKSIZE ((int)((double)ENTRY_CNT * PARAM_CNT / 32))

#define PATH_TO_FENS "../tune/zurichess_positions/quiet-labeled.epd"

#define TunepawnValue (1)
#define TuneknightValue (1)
#define TunebishopValue (1)
#define TunerookValue (1)
#define TunequeenValue (1)
#define TunepawnPSQT (1)
#define TuneknightPSQT (1)
#define TunebishopPSQT (1)
#define TunerookPSQT (1)
#define TunequeenPSQT (1)
#define TunekingPSQT (1)

#include "types.h"

typedef double Params[PARAM_CNT][PHASE_CNT];

struct TuneTuple {
    int coeff;
    int index;
};

struct TuneEntry {
    float result;
    int eval;
    int phase;
    int turn;

    double factors[PHASE_CNT];

    int ntuples;
    TuneTuple *tuples;
};

void runTexelTuning(int threadCnt);

#define INIT_PARAMS_0(term, S) do { \
    cparams[i][MG] = mgS(term); \
    cparams[i++][EG] = egS(term); \
} while(0)

#define INIT_PARAMS_1(term, A, S) do { \
    for (int _a = 0; _a < A; _a++){ \
        cparams[i][MG] = mgS(term[_a]); \
        cparams[i++][EG] = egS(term[_a]);} \
} while(0)

#define INIT_PARAMS_2(term, A, B, S) do { \
    for (int _b = 0; _b < A; _b++) \
        INIT_PARAMS_1(term[_b], B, S); \
} while(0)

#define INIT_COEFFS_0(term, S) do { \
    coeffs[i++] = T.term[WHITE] - T.term[BLACK]; \
} while(0)

#define INIT_COEFFS_1(term, A, S) do { \
    for (int _a = 0; _a < A; _a++) \
        coeffs[i++] = T.term[_a][WHITE] - T.term[_a][BLACK]; \
} while(0)

#define INIT_COEFFS_2(term, A, B, S) do { \
    for (int _b = 0; _b < A; _b++) \
        INIT_COEFFS_1(term[_b], B, S); \
} while(0)

#define PRINT_PARAMS_0(term, S) (print0(#term, tparams, i, S), i++)

#define PRINT_PARAMS_1(term, A, S) (print1(#term, tparams, i, A, S), i+=A)

#define PRINT_PARAMS_2(term, A, B, S) (print2(#term, tparams, i, A, B, S), i+=A*B)

#define ENABLE_0(F, term, S) do { \
    if (Tune##term) F##_0(term, S); \
} while(0)

#define ENABLE_1(F, term, A,  S) do { \
    if (Tune##term) F##_1(term, A, S); \
} while(0)

#define ENABLE_2(F, term, A, B, S) do { \
    if (Tune##term) F##_2(term, A, B, S); \
} while(0)

#define EXECUTE_ON_PARAMS(F) do { \
    ENABLE_0(F, pawnValue, ""); \
    ENABLE_0(F, knightValue, ""); \
    ENABLE_0(F, bishopValue, ""); \
    ENABLE_0(F, rookValue, ""); \
    ENABLE_0(F, queenValue, ""); \
    ENABLE_2(F, pawnPSQT, RANK_CNT, FILE_CNT/2, "[RANK_CNT][FILE_CNT/2]"); \
    ENABLE_2(F, knightPSQT, RANK_CNT, FILE_CNT/2, "[RANK_CNT][FILE_CNT/2]"); \
    ENABLE_2(F, bishopPSQT, RANK_CNT, FILE_CNT/2, "[RANK_CNT][FILE_CNT/2]"); \
    ENABLE_2(F, rookPSQT, RANK_CNT, FILE_CNT/2, "[RANK_CNT][FILE_CNT/2]"); \
    ENABLE_2(F, queenPSQT, RANK_CNT, FILE_CNT/2, "[RANK_CNT][FILE_CNT/2]"); \
    ENABLE_2(F, kingPSQT, RANK_CNT, FILE_CNT/2, "[RANK_CNT][FILE_CNT/2]"); \
} while(0)

#endif
