#pragma once

#include "types.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static Bitboard arrRectangular[SQ_CNT][SQ_CNT];

void initBitBoards();
void printBitBoard(Bitboard bb);

static const Bitboard notAFile = 0xFEFEFEFEFEFEFEFE;
static const Bitboard notABFile = 0xFCFCFCFCFCFCFCFC;
static const Bitboard notHFile = 0x7F7F7F7F7F7F7F7F;
static const Bitboard notGHFile = 0x3F3F3F3F3F3F3F3F;

static const Bitboard Rank1 = 0xFF;
static const Bitboard Rank2 = 0xFF00;
static const Bitboard Rank4 = 0xFF000000;
static const Bitboard Rank5 = 0xFF00000000;
static const Bitboard Rank7 = 0xFF000000000000;
static const Bitboard Rank8 = 0xFF00000000000000;
static const Bitboard FileA = 0x0101010101010101;

// Used to find pawn and king attacks
static inline Bitboard noeaOne(Bitboard bb) {return (bb << 7) & notHFile;}
static inline Bitboard nortOne(Bitboard bb) {return (bb << 8);}
static inline Bitboard noweOne(Bitboard bb) {return (bb << 9) & notAFile;}
static inline Bitboard westOne(Bitboard bb) {return (bb << 1) & notAFile;}
static inline Bitboard eastOne(Bitboard bb) {return (bb >> 1) & notHFile;}
static inline Bitboard soeaOne(Bitboard bb) {return (bb >> 9) & notHFile;}
static inline Bitboard soutOne(Bitboard bb) {return (bb >> 8);}
static inline Bitboard soweOne(Bitboard bb) {return (bb >> 7) & notAFile;}

// Used to find knight attacks
static inline Bitboard nonoea(Bitboard bb) {return (bb << 15) & notHFile;}
static inline Bitboard nonowe(Bitboard bb) {return (bb << 17) & notAFile;}
static inline Bitboard noeaea(Bitboard bb) {return (bb << 6) & notGHFile;}
static inline Bitboard nowewe(Bitboard bb) {return (bb << 10) & notABFile;}
static inline Bitboard sosoea(Bitboard bb) {return (bb >> 17) & notHFile;}
static inline Bitboard sosowe(Bitboard bb) {return (bb >> 15) & notAFile;}
static inline Bitboard soeaea(Bitboard bb) {return (bb >> 10) & notGHFile;}
static inline Bitboard sowewe(Bitboard bb) {return (bb >> 6) & notABFile;}



static inline Bitboard shift(Bitboard bb, int dir) {

    // Bitshift does not work with negative numbers
    // These numbers must be converted to positive numbers
    // then shifted the opposite direction

    switch (dir) {
        case 1:
        case -7:
        case 9:
            bb &= notHFile;
            break;
        case -1:
        case -9:
        case 7:
            bb &= notAFile;
            break;
    }

    return (dir > 0) ? (bb << dir) : (bb >> -dir);

}



static inline int popcnt(Bitboard bb) {

    // Built in function finds
    // amounts of bits in a bitboard

    return __builtin_popcountll(bb);

}



static inline bool validSquare(Square sq) {

    // If the square is in the range of 0-63

    return (sq >= 0) && (sq < 64);

}



static inline bool testBit(Bitboard bb, Square sq) {

    // Check the square is valid

    assert(validSquare(sq));

    // Checks to see if the bitboard
    // has a 1 bit at bit sq
    
    return bb & (1ULL << sq);

}



static inline void setBit(Bitboard *bb, Square sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // Tests if there is no bit at sq
    assert(!testBit(*bb, sq));

    // Since there is no bit at sq, we
    // can XOR the bit into the bitboard
    *bb ^= (1ULL << sq);

}



static inline void popBit(Bitboard *bb, Square sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // Tests if there is a bit at sq
    assert(testBit(*bb, sq));

    // Since there is a bit at sq, we
    // can XOR the bit into the bitboard
    *bb ^= (1ULL << sq);

}



static inline int getlsb(Bitboard bb) {
    // lsb stands for least significant bit

    // Checks to see if the bitboard
    // contains anything
    assert(bb);

    // Uses built in function
    return __builtin_ctzll(bb);

}



static inline int poplsb(Bitboard *bb) {

    // Checks to see if the bitboard
    // isn't empty
    assert(*bb);

    // Get the lsb of the bitboard
    int lsb = getlsb(*bb);

    // Pop the lsb
    *bb &= *bb - 1;

    // Return the lsb for use
    return lsb;

}



// A square can be anywhere from 0 to 63, which is 6 bits
// The file is always the first 3 bits
// The rank is always the last 3 bits
static inline int sq(File f, Rank r) {

    // Check file and rank are valid
    assert((f >= 0) && (f < 8));
    assert((r >= 0) && (r < 8));

    return (r << 3) | f;

}



static inline int file(Square sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // First 3 bits of a square will always
    // be the same as sq % 8

    return (sq & 7);

}



static inline int rank(Square sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // Last 3 bits of a square will always
    // be the same as sq / 8

    return (sq >> 3);

}



// Returns the square opposite to sq along the file axis
static inline int flipSq(Square square) {

    // Check the square is valid
    assert(validSquare(square));

    int r = rank(square);
    int f = 7 - file(square);
    return sq(f, r);

}



// Find the distance between two squares
static inline int distance(Square a, Square b) {
    
    // Check the squares are valid
    assert(validSquare(a));
    assert(validSquare(b));

    return MIN(abs(file(a) - file(b)), abs(rank(a) - rank(b)));

}


// Returns a Bitboard that contains just
// the specified rank
static inline Bitboard bbRank(Rank r) {

    // Check the rank is valid
    assert(r < 8);

    // Shifts a constant to return the Bitboard 
    // requested

    return Rank1 << (r * 8);

}



// Returns a Bitboard that contains just
// the specified file
static inline Bitboard bbFile(File f) {

    // Check the file is valid
    assert(file < 8);

    // Shifts a constant to return the Bitboard 
    // requested

    return FileA << f;

}

// Returns true if squares a b and c line up
// c is the bitboard of its square instead of an integer
static inline bool isBetween(Square a, Square b, Bitboard c) {
    return arrRectangular[a][b] & c;
}

// Returns a bitboard of the squares between square
// a and b
static inline Bitboard betweenBB(Square a, Square b) {
    return arrRectangular[a][b];
}

// Returns true if the BB contains more than 1 bit
static inline bool moreOneBit(Bitboard bb) {
    return bb & (bb - 1);
}
