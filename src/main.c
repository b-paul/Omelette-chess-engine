#include <stdio.h>

#include "attacks.h"
#include "bitboards.h"
#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "types.h"
#include "uci.h"

int main() {

    // Initialize program

    initAttacks();
    initBitBoards();
    initEval();
    initPosition();
    initSearch();
    initZobrist();

    // Run the UCI loop
    // Allows the engine to interact with a gui

    uciLoop();

    return 0;
}
