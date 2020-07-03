#include "attacks.h"
#include "bitboards.h"
#include "position.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>

// Directions for finding bishop and rook attacks
int bishopDir[4] = {9, 7, -7, -9};
int rookDir[4] = {8, 1, -1, -8};

// These fuctions just save the attacks of
// the pieces for each square to an array
void initPawnAttacks() {
    for (Square sq = 0; sq < SQ_CNT; sq++) {
        pawnAttacks[sq][WHITE] |= noeaOne(1ULL << sq);
        pawnAttacks[sq][WHITE] |= noweOne(1ULL << sq);
        pawnAttacks[sq][BLACK] |= soeaOne(1ULL << sq);
        pawnAttacks[sq][BLACK] |= soweOne(1ULL << sq);
    }
}

void initKnightAttacks() {
    for (Square sq = 0; sq < SQ_CNT; sq++) {
        knightAttacks[sq] |= nonoea(1ULL << sq);
        knightAttacks[sq] |= nonowe(1ULL << sq);
        knightAttacks[sq] |= noeaea(1ULL << sq);
        knightAttacks[sq] |= nowewe(1ULL << sq);
        knightAttacks[sq] |= soeaea(1ULL << sq);
        knightAttacks[sq] |= sowewe(1ULL << sq);
        knightAttacks[sq] |= sosoea(1ULL << sq);
        knightAttacks[sq] |= sosowe(1ULL << sq);
    }
}

void initKingAttacks() {
    for (Square sq = 0; sq < SQ_CNT; sq++) {
        kingAttacks[sq] |= noeaOne(1ULL << sq);
        kingAttacks[sq] |= nortOne(1ULL << sq);
        kingAttacks[sq] |= noweOne(1ULL << sq);
        kingAttacks[sq] |= eastOne(1ULL << sq);
        kingAttacks[sq] |= westOne(1ULL << sq);
        kingAttacks[sq] |= soeaOne(1ULL << sq);
        kingAttacks[sq] |= soutOne(1ULL << sq);
        kingAttacks[sq] |= soweOne(1ULL << sq);
    }
}

// This fuction gets the attack of a slider piece
// including the occupancy
// We dont use this in move generation etc, but
// instead save the results to an adress
Bitboard sliderAttack(Square sq, Bitboard occ, int Direction[4]) {
    Bitboard result = 0ULL;
    for (int i = 0; i < 4; i++) {
        for (Square square = sq + Direction[i]; validSquare(square) &&
        (distance(square, square - Direction[i]) == 1); square += Direction[i]) {
            setBit(&result, square);
            if (occ & (1ULL << square)) break;
        }
    }
    return result;
}

// This fuction saves the results of sliderAttack
// to an array, along with the magic number
// which is used to lookup the index using
// the function
// ((occ & mask) * magic) >> shift
void initSlider(Square sq, Magic* list, Bitboard magic, int Direction[4]) {
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


    for (Square sq = 0; sq < SQ_CNT; sq++) {
        // These arrays are a list of
        // attacks from slider pieces
        // excluding occupancy
        bPseudoAttacks[sq] = sliderAttack(sq, 0, bishopDir);
        rPseudoAttacks[sq] = sliderAttack(sq, 0, rookDir);

        initSlider(sq, bishopMagics, bishopMagic[sq], bishopDir);
        initSlider(sq, rookMagics, rookMagic[sq], rookDir);
    }
}
