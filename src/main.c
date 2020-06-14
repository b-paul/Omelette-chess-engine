#include <stdio.h>
#include <string.h>

#include "attacks.h"
#include "bitboards.h"
#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
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

    printf("%d\n", mgS(PSQT[3][42]));

    // Check for a bench used in
    // OpenBench
    if (argc > 1 && strstr(argv[1], "bench") == argv[1]) {
        bench(argc, argv);
        return 0;
    }

    // Run the UCI loop
    // Allows the engine to interact with a gui

    uciLoop();

    return 0;
}
