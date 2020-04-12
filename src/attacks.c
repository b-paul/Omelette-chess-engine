#include "attacks.h"
#include "bitboards.h"
#include "position.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>

// Directions for finding bishop and rook attacks
int bishopDir[4] = {9, 7, -7, -9};
int rookDir[4] = {8, 1, -1, -8};

void initPawnAttacks() {
    for (int i = 0; i < SQ_CNT; i++) {
        pawnAttacks[i][WHITE] |= noeaOne(1ULL << i);
        pawnAttacks[i][WHITE] |= noweOne(1ULL << i);
        pawnAttacks[i][BLACK] |= soeaOne(1ULL << i);
        pawnAttacks[i][BLACK] |= soweOne(1ULL << i);
    }
}

void initKnightAttacks() {
    for (int i = 0; i < SQ_CNT; i++) {
        knightAttacks[i] |= nonoea(1ULL << i);
        knightAttacks[i] |= nonowe(1ULL << i);
        knightAttacks[i] |= noeaea(1ULL << i);
        knightAttacks[i] |= nowewe(1ULL << i);
        knightAttacks[i] |= soeaea(1ULL << i);
        knightAttacks[i] |= sowewe(1ULL << i);
        knightAttacks[i] |= sosoea(1ULL << i);
        knightAttacks[i] |= sosowe(1ULL << i);
    }
}

void initKingAttacks() {
    for (int i = 0; i < SQ_CNT; i++) {
        kingAttacks[i] |= noeaOne(1ULL << i);
        kingAttacks[i] |= nortOne(1ULL << i);
        kingAttacks[i] |= noweOne(1ULL << i);
        kingAttacks[i] |= eastOne(1ULL << i);
        kingAttacks[i] |= westOne(1ULL << i);
        kingAttacks[i] |= soeaOne(1ULL << i);
        kingAttacks[i] |= soutOne(1ULL << i);
        kingAttacks[i] |= soweOne(1ULL << i);
    }
}

Bitboard sliderAttack(int sq, Bitboard occ, int Direction[4]) {
    Bitboard result = 0ULL;
    for (int i = 0; i < 4; i++) {
        for (int square = sq + Direction[i]; validSquare(square) &&
        (distance(square, square - Direction[i]) == 1); square += Direction[i]) {
            setBit(&result, square);
            if (occ & (1ULL << square)) break;
        }
    }
    return result;
}

void initSlider(int sq, Magic* list, Bitboard magic, int Direction[4]) {
    // Used with the mask
    Bitboard edges = ((bbRank(RANK_1) | bbRank(RANK_8)) & ~bbRank(rank(sq)))
                                     | ((bbFile(FILE_A) | bbFile(FILE_H)) & ~bbFile(file(sq)));

    Bitboard occ = 0ULL;

    // Store values
    list[sq].magic = magic;
    list[sq].mask = sliderAttack(sq, occ, Direction) & ~edges;
    list[sq].shift = 64 - popcnt(list[sq].mask);

    // If this is not the last square then assign memory for the next
    // square
    // First square is defined in the initAttacks function
    if (sq != H8)
        list[sq + 1].attacks = list[sq].attacks + (1ULL << popcnt(list[sq].mask));

    // Loops through all possible occupancies
    do {
        list[sq].attacks[sliderIndex(occ, list[sq])] = sliderAttack(sq, occ, Direction);
        occ = (occ - list[sq].mask) & list[sq].mask;
    } while (occ);
}

void initAttacks() {
    initPawnAttacks();
    initKnightAttacks();
    initKingAttacks();

    bishopMagics[A1].attacks = bishopAttacks;
    rookMagics[A1].attacks = rookAttacks;


    for (int i = 0; i < SQ_CNT; i++) {
        bPseudoAttacks[i] = sliderAttack(i, 0, bishopDir);
        rPseudoAttacks[i] = sliderAttack(i, 0, rookDir);
        initSlider(i, bishopMagics, bishopMagic[i], bishopDir);
        initSlider(i, rookMagics, rookMagic[i], rookDir);
    }
}
