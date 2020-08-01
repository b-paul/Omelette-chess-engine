// A lot of the code for tuning is from ethereal
// Credit to them
// I decided to do this over a python script because
// I like having everything internal

#ifdef TUNE

#include "evaluate.h"
#include "position.h"
#include "threads.h"
#include "tune.h"
#include "types.h"
#include "uci.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int pawnValue;
extern int knightValue;
extern int bishopValue;
extern int rookValue;
extern int queenValue;

extern int pawnPSQT[RANK_CNT][FILE_CNT/2];
extern int knightPSQT[RANK_CNT][FILE_CNT/2];
extern int bishopPSQT[RANK_CNT][FILE_CNT/2];
extern int rookPSQT[RANK_CNT][FILE_CNT/2];
extern int queenPSQT[RANK_CNT][FILE_CNT/2];
extern int kingPSQT[RANK_CNT][FILE_CNT/2];

extern int knightMobilityBonus[9];
extern int bishopMobilityBonus[14];
extern int rookMobilityBonus[15];
extern int queenMobilityBonus[28];

extern int bishopPairBonus;

extern EvalTrace T, emptyTrace;

TuneTuple *TupleStack;
int TupleStackSize = STACKSIZE;

void print0(char *name, int params[PARAM_CNT][PHASE_CNT], int i, char *S) {
    printf("int %s%s = S(%d, %d);\n\n", name, S, params[i][MG], params[i][EG]);
}

void print1(char *name, int params[PARAM_CNT][PHASE_CNT], int i, int A, char *S) {
    printf("int %s%s = {\n    ", name, S);
    for (int a = 0; a < A; a++, i++) {
        printf("S(%d, %d), ", params[i][MG], params[i][EG]);
        if (a % 8 == 7) printf("\n    ");
    }
    printf("\n};\n\n");
}

void print2(char *name, int params[PARAM_CNT][PHASE_CNT], int i, int A, int B, char *S) {
    printf("int %s%s = {\n", name, S);

    for (int a = 0; a < A; a++) {
        printf("    {");

        for (int b = 0; b < B; b++, i++)
            printf("S(%d, %d), ", params[i][MG], params[i][EG]);

        printf("},\n");
    }

    printf("\n};\n\n");
}

void printParams(Params params, Params cparams) {
    int tparams[PARAM_CNT][PHASE_CNT];

    for (int j = 0; j < PARAM_CNT; j++)
        for (int k = MG; k <= EG; k++)
            tparams[j][k] = params[j][k];

    int i = 0;
    EXECUTE_ON_PARAMS(PRINT_PARAMS);

    if (i != PARAM_CNT) {
        printf("error printing parameters\n");
        exit(0);
    }
}

double sigmoid(double S, double K) {
    return 1.0/(1.0+exp(-K*S/400.0));
}

double newEval(TuneEntry *entry, Params params) {
    double mg = 0, eg = 0;
    for (int i = 0; i < entry->ntuples; i++) {
        mg += params[entry->tuples[i].index][MG] * entry->tuples[i].coeff;
        eg += params[entry->tuples[i].index][EG] * entry->tuples[i].coeff;
    }
    return entry->eval + ((mg * (256 - entry->phase)) + (eg * entry->phase))/256.0;
}

double newSingleError(TuneEntry *entry, Params params, double K) {
    double s = sigmoid(newEval(entry, params), K);
    double sprime = s * (1-s);
    return (entry->result - s) * sprime;
}

double newFullError(TuneEntry *entries, Params params, double K) {
    double r = 0.0;
    #pragma omp parallel shared(r)
    {
        #pragma omp for schedule(static, ENTRY_CNT/PARTITION_CNT) reduction(+:r)
        for (int i = 0; i < ENTRY_CNT; i++)
            r += SQUARED(entries[i].result - sigmoid(newEval(&entries[i], params), K));
    }
    return r/(double)ENTRY_CNT;
}

double fullError(TuneEntry *entries, double K) {
    double r = 0.0;
    #pragma omp parallel shared(r)
    {
        #pragma omp for schedule(static, ENTRY_CNT/PARTITION_CNT) reduction(+:r)
        for (int i = 0; i < ENTRY_CNT; i++)
            r += SQUARED(entries[i].result - sigmoid(entries[i].eval, K));
    }
    return r/(double)ENTRY_CNT;
}

double computeK(TuneEntry *entries, Params params) {
    double start = -10.0, end = 10.0, delta = 1.0;
    double curr = start, err, best = newFullError(entries, params, start);

    for (int i = 0; i < K_PRECISION; i++) {
        curr = start - delta;

        while (curr < end) {
            curr += delta;
            err = newFullError(entries, params, curr);
            if (err <= best) {
                best = err;
                start = curr;
            }
        }

        end = start + delta;
        start -= delta;
        delta /= 10.0;
    }

    return start;
}

void shuffleEntries(TuneEntry *entries) {
    TuneEntry temp;

    for (int i = 0; i < ENTRY_CNT; i++) {
        int r = (randBB() % (ENTRY_CNT - i)) + i;

        temp = entries[i];
        entries[i] = entries[r];
        entries[r] = temp;
    }
}

void updateGradient(TuneEntry *entries, Params gradient, Params params, double K, int batch) {

    #pragma omp parallel shared(gradient)
    {
        Params local = {0};
        #pragma omp for schedule(static, BATCH_SIZE/PARTITION_CNT)
        for (int i = batch * BATCH_SIZE; i < (batch + 1) * BATCH_SIZE; i++) {
            double err = newSingleError(&entries[i], params, K);

            for (int j = 0; j < entries[i].ntuples; j++)
                for (int k = MG; k <= EG; k++)
                    local[entries[i].tuples[j].index][k] += err * entries[i].factors[k] * entries[i].tuples[j].coeff;
        }

        for (int i = 0; i < PARAM_CNT; i++)
            for (int j = MG; j <= EG; j++)
                gradient[i][j] += local[i][j];
    }
}

void updateMemory(TuneEntry *entry, int size) {

    if (size > TupleStackSize) {
        TupleStackSize = STACKSIZE;
        TupleStack = calloc(STACKSIZE, sizeof(TuneTuple));
    }

    entry->tuples = TupleStack;
    entry->ntuples = size;

    TupleStack += size;
    TupleStackSize -= size;
}

void initCoeffs(int coeffs[PARAM_CNT]) {
    int i = 0;

    EXECUTE_ON_PARAMS(INIT_COEFFS);

    if (i != PARAM_CNT) {
        printf("error initializing coefficients\n");
        exit(0);
    }
}

void initParams(Params cparams) {
    int i = 0;

    EXECUTE_ON_PARAMS(INIT_PARAMS);

    if (i != PARAM_CNT) {
        printf("error initializing paramaters\n");
        exit(0);
    }
}

void initEntries(TuneEntry *entries, Thread *thread) {
    char str[256];
    FILE *f = fopen(PATH_TO_FENS, "r");
    Pos board;
    int i,j,k, coeffs[PARAM_CNT];
    for (i = 0; i < ENTRY_CNT; i++) {
        if (fgets(str, 256, f) == NULL) {
            printf("Error reading line %d from file %s\n", i, PATH_TO_FENS);
            exit(1);
        }
        resetBoard(&board);
        parseFen(str, &board);

        T = emptyTrace;
        entries[i].eval = evaluate(&board);
        entries[i].eval = board.turn == WHITE ? entries->eval : -entries->eval;

        entries[i].phase = 24;
        entries[i].phase -= popcnt(board.pieces[KNIGHT] | board.pieces[BISHOP]);
        entries[i].phase -= popcnt(board.pieces[ROOK]) * 2;
        entries[i].phase -= popcnt(board.pieces[QUEEN]) * 4;

        entries[i].factors[MG] = 1 - entries[i].phase / 24.0;
        entries[i].factors[EG] = entries[i].phase / 24.0;

        entries[i].phase = (entries[i].phase * 256 + 12)/24.0;

        if (strstr(str, "\"1-0\"")) entries[i].result = 1.0;
        else if (strstr(str, "\"1/2-1/2\"")) entries[i].result = 0.5;
        else if (strstr(str, "\"0-1\"")) entries[i].result = 0.0;
        else {
            printf("Invalid score at line %d of file %s\n", i, PATH_TO_FENS);
            exit(0);
        }

        initCoeffs(coeffs);

        for (j = 0, k = 0; j < PARAM_CNT; j++) {
            k += coeffs[j] != 0;
        }

        updateMemory(&entries[i], k);

        for (j = 0, k = 0; j < PARAM_CNT; j++) {
            if (coeffs[j] != 0) {
                entries[i].tuples[k].index = j;
                entries[i].tuples[k++].coeff = coeffs[j];
            }
        }
    }
    fclose(f);
}

void runTexelTuning(int threadCnt) {
    HistoryTable hTable;
    initHistoryTable(&hTable);
    tTable tt;
    initTT(&tt, 1);
    Thread *threads = initThreads(threadCnt, &tt, &hTable);

    TuneEntry *entries = (TuneEntry*)calloc(ENTRY_CNT, sizeof(TuneEntry));
    Params cparams, params = {0};

    double error, best = 999999, rate = LEARNING_RATE;

    // Init rng
    srand(time(0));

    TupleStack = calloc(STACKSIZE, sizeof(TuneTuple));

    initParams(cparams);
    initParams(params);

    initEntries(entries, threads);

    const double K = computeK(entries, params);
    //const double K = 0.832;

    printf("K = %g\n", K);

    printf("Starting error %g\n", newFullError(entries, params, K));

    int iterations = 0;

    while (1) {
        iterations++;

        if ((iterations % REPORTING) == 0) {

            error = newFullError(entries, params, K);
            if (error > best) rate /= LR_DROP_RATE;

            best = error;
            printParams(params, cparams);
            printf("Iteration %d Error %g\n\n\n", iterations, best);
        }

        shuffleEntries(entries);

        for (int batch = 0; batch < ENTRY_CNT/BATCH_SIZE; batch++) {

            Params gradient = {0};
            updateGradient(entries, gradient, params, K, batch);
            for (int i = 0; i < PARAM_CNT; i++)
                for (int j = MG; j <= EG; j++)
                    params[i][j] += (2.0 / BATCH_SIZE) * rate * gradient[i][j];
        }
    }
}

#endif
