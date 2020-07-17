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

extern int materialBonus[PIECE_TYPE_CNT];
extern int pawnPSQT[RANK_CNT][FILE_CNT/2];
extern int knightPSQT[RANK_CNT][FILE_CNT/2];
extern int bishopPSQT[RANK_CNT][FILE_CNT/2];
extern int rookPSQT[RANK_CNT][FILE_CNT/2];
extern int queenPSQT[RANK_CNT][FILE_CNT/2];
extern int kingPSQT[RANK_CNT][FILE_CNT/2];

void printParams(Params params) {
    printf("int materialBonus[PIECE_TYPE_CNT] = {\n");
    printf("    0,\n");
    for (int i = 0; i < 5; i++) {
        printf("    S(%d, %d),\n", 
                mgS(materialBonus[i+1]) + (int)params[i][MG], 
                egS(materialBonus[i+1]) + (int)params[i][EG]);
    }
    printf("    S(0, 0)\n};\n\n");
/*
    printf("int PSQTBonus[PIECE_TYPE_CNT][RANK_CNT][FILE_CNT/2] = {\n");
    printf("    {\n");
    for (int p = 1; p < 6; p++) {
        printf("    }, {\n");
        for (int r = 0; r < RANK_CNT; r++) {
            printf("    {");
            for (int f = 0; f < FILE_CNT/2; f++)
                printf("S(%d, %d), ",
                        mgS(PSQTBonus[p][r][f]) + (int)params[5+(32*p)+(4*r)+f][MG],
                        egS(PSQTBonus[p][r][f]) + (int)params[5+(32*p)+(4*r)+f][EG]);
            printf("},\n");
        }
    }
    printf("};\n");
*/
}

double sigmoid(double S, double K) {
    return 1.0/(1.0+exp(-K*S/400.0));
}

int newEval(TuneEntry *entry, Params params) {
    int mg = 0, eg = 0;
    for (int i = 0; i < PARAM_CNT; i++) {
        //mg += params[i][MG] * entry->evalDiff[i][MG];
        //eg += params[i][EG] * entry->evalDiff[i][EG];
    }
    int result = ((mg * (256 - entry->phase)) + (eg * (entry->phase)))/256;
    return entry->eval + (entry->turn == WHITE ? result : -result);
}

double newSingleError(TuneEntry *entry, Params params, double K) {
    double s = sigmoid(K, newEval(entry, params));
    double sPrime = s * (1-s);
    return (entry->result - s) * sPrime;
}

double newFullError(TuneEntry *entries, Params params, double K) {
    double r = 0;
    for (int i = 0; i < ENTRY_CNT; i++) {
        r += SQUARED(entries[i].result - sigmoid(newEval(&entries[i], params), K));
    }
    return r/ENTRY_CNT;
}

double fullError(TuneEntry *entries, double K) {
    double r = 0;
    for (int i = 0; i < ENTRY_CNT; i++) {
        r += SQUARED(entries[i].result - sigmoid(entries[i].eval, K));
    }
    return r/ENTRY_CNT;
}

double computeK(TuneEntry *entries) {
    double start = -10.0, end = 10.0, delta = 1.0;
    double curr = start, err , best = fullError(entries, start);

    for (int i = 0; i < K_PRECISION; i++) {
        curr = start - delta;

        while (curr < end) {
            curr += delta;
            err = fullError(entries, curr);
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

int getParam(int paramIdx) {
    assert(paramIdx > PARAM_CNT);
    if (paramIdx - 5 < 0) {
        return materialBonus[paramIdx+1];
    }
    paramIdx -= 5;
    if (paramIdx - 192 < 0) {
        int piece = (paramIdx / 32) + 1;
        int file = (paramIdx % 32) / 4;
        int rank = (paramIdx % 4);
        return PSQTBonus[piece][rank][file];
    }

    // If something goes wrong, return pawn material
    assert(0);
    return materialBonus[PAWN];
}

void updateParams(TuneEntry *entries, Params params, double K, int batch) {
    Params grad = {0};
    double err;

    // Iterate over each position in the mini-batch
    for (int i = batch * BATCH_SIZE; i < (batch + 1); i++) {
        err = newSingleError(&entries[i], params, K);

        for (int j = 0; j < PARAM_CNT; j++) {
            for (int k = MG; k <= MG; k++) {
                //if (entries[i].evalDiff[j][k] == 0) continue;
                //grad[j][k] += err * entries[i].factors[k] * entries[i].evalDiff[j][k];
            }
        }
    }

    for (int i = 0; i < PARAM_CNT; i++) {
        for (int j = MG; j <= MG; j++) {
            params[i][j] += (2.0 / BATCH_SIZE) * LEARNING_RATE * grad[i][j];
        }
    }
}

void updateRealEvalParam(int index, int value) {
    if (index - 5 < 0) {
        materialBonus[index+1] = value;
        return;
    }
    index -= 5;
    if (index - 192 < 0) {
        int piece = (index / 32) + 1;
        int file = (index % 32) / 4;
        int rank = (index % 4);
        PSQTBonus[piece][rank][file] = value;
        return;
    }

}

void initEntries(TuneEntry *entries, Thread *thread) {
    char str[256];
    FILE *f = fopen(PATH_TO_FENS, "r");
    Pos board;
    int i,j;
    for (i = 0; i < ENTRY_CNT; i++) {
        if (fgets(str, 256, f) == NULL) {
            printf("Error reading line %d from file %s\n", i, PATH_TO_FENS);
            exit(1);
        }
        resetBoard(&board);
        parseFen(str, &board);
        entries[i].eval = evaluate(&board);
        entries[i].phase = phase(&board);
        entries[i].turn = board.turn;

        entries[i].factors[MG] = 1 - entries[i].phase / 24.0;
        entries[i].factors[EG] = entries[i].phase / 24.0;

        if (strstr(str, "[1.0]")) entries[i].result = 1.0;
        else if (strstr(str, "[0.5]")) entries[i].result = 0.5;
        else if (strstr(str, "[0.0]")) entries[i].result = 0.0;
        else {
            printf("Invalid score at line %d of file %s\n", i, PATH_TO_FENS);
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
    Params params = {0};

    // Init rng
    srand(time(0));

    initEntries(entries, threads);

    const double K = computeK(entries);

    int iterations = 0;

    while (1) {
        iterations++;

        if ((iterations % 100) == 0) printParams(params);
        printf("Error=%lf\n", newFullError(entries, params, K));

        shuffleEntries(entries);

        // Loop through all positions in batches
        for (int batch = 0; batch < ENTRY_CNT/BATCH_SIZE; batch++) {

            updateParams(entries, params, K, batch);

        }
    }
}

#endif
