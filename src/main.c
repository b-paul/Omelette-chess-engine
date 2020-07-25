#include <stdio.h>
#include <string.h>

#include "attacks.h"
#include "bitboards.h"
#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "tune.h"
#include "types.h"
#include "uci.h"

int main(int argc, char **argv) {

    // Initialize program

    initAttacks();
    initBitBoards();
    initEval();
    initPosition();
    initSearch();
    initZobrist();

    // Check for a bench used in
    // OpenBench
    if (argc > 1 && strstr(argv[1], "bench") == argv[1]) {
        bench(argc, argv);
        return 0;
    }
#ifdef TUNE
    if (argc > 1 && strstr(argv[1], "tune") == argv[1]) {
        runTexelTuning(argc > 2 ? atoi(argv[2]) : 1);
        return 0;
    }
#endif

    // Run the UCI loop
    // Allows the engine to interact with a gui

    uciLoop();

    return 0;
}
