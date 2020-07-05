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

double sigmoid(double S, double K) {
    return 1.0/(1.0+exp(-K*S/400.0));
}

double fullError(TuneEntry *entries, double K) {
    double r = 0;
    for (int i = 0; i < ENTRY_CNT; i++) {
        r += SQUARED(entries[i].result - sigmoid(entries[i].eval, K));
    }
    return (1.0/ENTRY_CNT) * r;
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

void initEntries(TuneEntry *entries, Thread *thread) {
    char str[256];
    FILE *f = fopen(PATH_TO_FENS, "r");
    Pos board;
    for (int i = 0; i < ENTRY_CNT; i++) {
        if (fgets(str, 256, f) == NULL) {
            printf("Error reading line %d from file %s\n", i, PATH_TO_FENS);
            exit(1);
        }
        resetBoard(&board);
        parseFen(str, &board);
        entries[i].eval = evaluate(&board);

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

    initEntries(entries, threads);
}

#endif
