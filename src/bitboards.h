#pragma once

#include "types.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static Bitboard arrRectangular[SQ_CNT][SQ_CNT];

void initBitBoards();
void printBitBoard(const Bitboard bb);

static const Bitboard notAFile = 0xFEFEFEFEFEFEFEFE;
static const Bitboard notABFile = 0xFCFCFCFCFCFCFCFC;
static const Bitboard notHFile = 0x7F7F7F7F7F7F7F7F;
static const Bitboard notGHFile = 0x3F3F3F3F3F3F3F3F;

static const Bitboard LightSquares = 0x55AA55AA55AA55AA;
static const Bitboard DarkSquares = 0xAA55AA55AA55AA55;

static const Bitboard Rank1 = 0xFF;
static const Bitboard Rank2 = 0xFF00;
static const Bitboard Rank4 = 0xFF000000;
static const Bitboard Rank5 = 0xFF00000000;
static const Bitboard Rank7 = 0xFF000000000000;
static const Bitboard Rank8 = 0xFF00000000000000;
static const Bitboard FileA = 0x0101010101010101;

// Used to find pawn and king attacks
static inline Bitboard noeaOne(const Bitboard bb) {return (bb << 7) & notHFile;}
static inline Bitboard nortOne(const Bitboard bb) {return (bb << 8);}
static inline Bitboard noweOne(const Bitboard bb) {return (bb << 9) & notAFile;}
static inline Bitboard westOne(const Bitboard bb) {return (bb << 1) & notAFile;}
static inline Bitboard eastOne(const Bitboard bb) {return (bb >> 1) & notHFile;}
static inline Bitboard soeaOne(const Bitboard bb) {return (bb >> 9) & notHFile;}
static inline Bitboard soutOne(const Bitboard bb) {return (bb >> 8);}
static inline Bitboard soweOne(const Bitboard bb) {return (bb >> 7) & notAFile;}

// Used to find knight attacks
static inline Bitboard nonoea(const Bitboard bb) {return (bb << 15) & notHFile;}
static inline Bitboard nonowe(const Bitboard bb) {return (bb << 17) & notAFile;}
static inline Bitboard noeaea(const Bitboard bb) {return (bb << 6) & notGHFile;}
static inline Bitboard nowewe(const Bitboard bb) {return (bb << 10) & notABFile;}
static inline Bitboard sosoea(const Bitboard bb) {return (bb >> 17) & notHFile;}
static inline Bitboard sosowe(const Bitboard bb) {return (bb >> 15) & notAFile;}
static inline Bitboard soeaea(const Bitboard bb) {return (bb >> 10) & notGHFile;}
static inline Bitboard sowewe(const Bitboard bb) {return (bb >> 6) & notABFile;}



static inline Bitboard shift(Bitboard bb, const int dir) {

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



static inline int popcnt(const Bitboard bb) {

    // Built in function finds
    // amounts of bits in a bitboard

    return __builtin_popcountll(bb);

}



static inline bool validSquare(const int sq) {

    // If the square is in the range of 0-63

    return (sq >= 0) && (sq < 64);

}



static inline bool testBit(const Bitboard bb, const int sq) {

    // Check the square is valid

    assert(validSquare(sq));

    // Checks to see if the bitboard
    // has a 1 bit at bit sq
    
    return bb & (1ULL << sq);

}



static inline void setBit(Bitboard *bb, const int sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // Tests if there is no bit at sq
    assert(!testBit(*bb, sq));

    // Since there is no bit at sq, we
    // can XOR the bit into the bitboard
    *bb ^= (1ULL << sq);

}



static inline void popBit(Bitboard *bb, const int sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // Tests if there is a bit at sq
    assert(testBit(*bb, sq));

    // Since there is a bit at sq, we
    // can XOR the bit into the bitboard
    *bb ^= (1ULL << sq);

}



// Get the least significant bit in a bitboard
static inline int getlsb(const Bitboard bb) {

    // Checks to see if the bitboard
    // contains anything
    assert(bb);

    // Uses built in function
    return __builtin_ctzll(bb);

}



// Pop and return the least significant bit in a bitboard
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



// Returns a square based on the file and rank
// A square can be anywhere from 0 to 63, which is 6 bits
// The file is always the first 3 bits
// The rank is always the last 3 bits
static inline int sq(const int r, const int f) {

    // Check file and rank are valid
    assert((f >= 0) && (f < 8));
    assert((r >= 0) && (r < 8));

    return (r << 3) | f;

}



// Return the file of a square
static inline int file(const int sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // First 3 bits of a square will always
    // be the same as sq % 8

    return (sq & 7);

}



// Returns the rank of a square
static inline int rank(const int sq) {

    // Check the square is valid
    assert(validSquare(sq));

    // Last 3 bits of a square will always
    // be the same as sq / 8

    return (sq >> 3);

}



// Returns the square opposite to sq along the file axis
static inline int flipSq(const int square) {

    // Check the square is valid
    assert(validSquare(square));

    return sq(rank(square), 7 - file(square));

}



// Find the distance between two squares
static inline int distance(const int a, const int b) {
    
    // Check the squares are valid
    assert(validSquare(a));
    assert(validSquare(b));

    return MAX(abs(file(a) - file(b)), abs(rank(a) - rank(b)));

}


// Returns a Bitboard that contains just
// the specified rank
static inline Bitboard bbRank(const int rank) {

    // Check the rank is valid
    assert(rank < 8);

    // Shifts a constant to return the Bitboard 
    // requested

    return Rank1 << (rank * 8);

}



// Returns a Bitboard that contains just
// the specified file
static inline Bitboard bbFile(const int file) {

    // Check the file is valid
    assert(file < 8);

    // Shifts a constant to return the Bitboard 
    // requested

    return FileA << file;

}

// Returns true if squares a b and c line up
// c is the bitboard of its square instead of an integer
static inline int isBetween(const int a, const int b, const Bitboard c) {
    return arrRectangular[a][b] & c;
}

// Returns a bitboard of the squares between square
// a and b
static inline int betweenBB(const int a, const int b) {
    return arrRectangular[a][b];
}

// Returns true if the BB contains more than 1 bit
static inline int multipleBits(const Bitboard bb) {
    return bb & (bb - 1);
}
