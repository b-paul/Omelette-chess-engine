#include "types.h"
#include "bitboards.h"
#include "attacks.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// Initialises bitboards used in other functions
void initBitBoards() {
    for (int i = 0; i < SQ_CNT; i++) {
        for (int j = 0; j < SQ_CNT; j++) {
            if (getBishopAttacks(i, 0ULL) & (1ULL << j)) {
                arrRectangular[i][j] = (getBishopAttacks(i, 0) &
                                                             getBishopAttacks(j, 0)) | (1ULL << i) | (1ULL << j);
            } else if (getRookAttacks(i, 0ULL) & (1ULL << j)) {
                arrRectangular[i][j] = (getRookAttacks(i, 0) &
                                                             getRookAttacks(j, 0)) | (1ULL << i) | (1ULL << j);
            }
        }
    }
}

void printBitBoard(Bitboard bb) {
    // Loop through each square to print
    
    // Loop through rank backwards since we want the 8th rank
    // to be at the top

    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file < FILE_CNT; file++) {
            // Print X if there is a bit at sq, else print -
            printf("%s", (testBit(bb, sq(file, rank)) ? "X" : "-"));
        }
        
        // Line gap between ranks

        printf("\n");
    }
    printf("\n\n");
}
