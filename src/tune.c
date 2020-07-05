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
extern int PSQTBonus[PIECE_TYPE_CNT][RANK_CNT][FILE_CNT/2];

double sigmoid(double S, double K) {
    return 1.0/(1.0+exp(-K*S/400.0));
}

int newEval(TuneEntry *entry, int params[PARAM_CNT][PHASE_CNT]) {
    int mg = 0, eg = 0;
    for (int i = 0; i < PARAM_CNT; i++) {
        mg += params[i][MG] * entry->evalDiff[i][MG];
        eg += params[i][EG] * entry->evalDiff[i][EG];
    }
    int result = ((mg * (256 - entry->phase)) + (eg * (entry->phase)))/256;
    return entry->eval + (board->turn == WHITE ? result : -result);
}

double newFullError(TuneEntry *entries, int params[PARAM_CNT][PHASE_CNT], double K) {
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
        int r = randBB() % ENTRY_CNT;

        temp = entries[i];
        entries[i] = entries[r];
        entries[r] = temp;
    }
}

int *getParam(int paramIdx) {
    assert(paramIdx > PARAM_CNT);
    if (paramIdx - 5 < 0) {
        return &materialBonus[paramIdx+1];
    }
    paramIdx -= 5;
    if (paramIdx - 192 < 0) {
        int piece = (paramIdx / 32) + 1;
        int file = (paramIdx % 32) / 4;
        int rank = (paramIdx % 4);
        return &PSQTBonus[piece][rank][file];
    }

    // If something goes wrong, return pawn material
    assert(0);
    return &materialBonus[PAWN];
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

        if (strstr(str, "[1.0]")) entries[i].result = 1.0;
        else if (strstr(str, "[0.5]")) entries[i].result = 0.5;
        else if (strstr(str, "[0.0]")) entries[i].result = 0.0;
        else {
            printf("Invalid score at line %d of file %s\n", i, PATH_TO_FENS);
        }

        for (j = 0; j < PARAM_CNT; j++) {
            int *param = getParam(j);
            *param = S(mgS(*param)+1, egS(*param));
            entries[i].evalDiff[j][MG] = entries[i].eval - evaluate(&board);
            *param = S(mgS(*param)-1, egS(*param)+1);
            entries[i].evalDiff[j][EG] = entries[i].eval - evaluate(&board);
            *param = S(mgS(*param), egS(*param)-1);
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
    int params[PARAM_CNT][PHASE_CNT];

    // Init rng
    srand(time(0));

    initEntries(entries, threads);

    const double k = computeK(entries);

    while (1) {
        shuffleEntries(entries);

        // Loop through all positions in batches
        for (int batch = 0; batch < ENTRY_CNT/BATCH_SIZE; batch++) {
        }
    }
}

#endif
