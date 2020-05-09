#include "bitboards.h"
#include "position.h"
#include "types.h"

#include <stdint.h>

#define S(mg, eg) ((int)((unsigned int)(eg) << 16) + mg)
#define mgS(mg) ((int16_t)((uint16_t)((unsigned)(mg) & 0xFFFF)))
#define egS(eg) ((int16_t)((uint16_t)(((unsigned)(eg) + 0x8000)>>16)))

int materialBonus[PIECE_CNT] = {
    0,
    S(100, 150), // Pawns
    S(300, 250), // Knights
    S(325, 400), // Bishops
    S(500, 600), // Rooks
    S(900, 1100) // Queens
};

int PSQTBonus[PIECE_CNT][RANK_CNT][FILE_CNT/2] = {
    {},
    { // Pawns
        {S(0,0), S(0,0), S(0,0), S(0,0)},
        {S(10,-10), S(-5,-10), S(-8,-10), S(-20,-10)},
        {S(0,-10), S(-2,-10), S(2,-10), S(-5,-10)},
        {S(-5,-6), S(-5,-6), S(10,-6), S(25,-6)},
        {S(-5,0), S(-5,0), S(13,0), S(30,0)},
        {S(12,30), S(12,30), S(18,30), S(35,30)},
        {S(50,50), S(50,50), S(50,50), S(50,50)},
        {S(0,0), S(0,0), S(0,0), S(0,0)},
    }, { // Knights
        {S(-20,-10), S(-10,-10), S(-10,-10), S(-10,-10)},
        {S(-15,-10), S(-5,-5), S(-5,-5), S(10,0)},
        {S(-15,-10), S(-5,-5), S(30,5), S(25,10)},
        {S(-10,-10), S(-3,-4), S(20,20), S(30,30)},
        {S(-10,-10), S(-5,-4), S(25,20), S(40,30)},
        {S(-10,-10), S(-5,-5), S(20,5), S(20,0)},
        {S(-10,-10), S(-5,-5), S(-10,-50), S(-10,10)},
        {S(-10,-10), S(-10,-10), S(-10,-10), S(-10,-10)},
    }, { // Bishops
        {S(-20,-10), S(-18,-10), S(-10,-10), S(-18,-10)},
        {S(-20,-10), S(25,-8), S(-8,-8), S(15,-8)},
        {S(-20,-10), S(-5,-8), S(10,0), S(20,0)},
        {S(-20,-10), S(2,-8), S(25,0), S(30,8)},
        {S(-20,-10), S(25,-8), S(-4,0), S(23,8)},
        {S(-20,-10), S(-5,-8), S(8,0), S(10,0)},
        {S(-20,-10), S(-5,-8), S(-5,-8), S(-5,0)},
        {S(-20,-10), S(-20,-10), S(-20,-10), S(-20,-10)},
    }, { // Rooks
        {S(-30,-20), S(-30,-18), S(5,-17), S(15,-15)},
        {S(-15,-18), S(-20,-16), S(-10,-15), S(5,-13)},
        {S(-10,-16), S(-9,-14), S(-5,-13), S(3,-11)},
        {S(-10,-14), S(-9,-12), S(-5,-11), S(3,-9)},
        {S(-5,-12), S(-4,-10), S(0,-9), S(8,-7)},
        {S(8,0), S(6,0), S(4,0), S(1,0)},
        {S(15,20), S(15,20), S(15,20), S(15,20)},
        {S(8,16), S(12,16), S(15,16), S(18,16)},
    }, { // Queens
        {S(4,-50), S(4,-50), S(4,-50), S(100,-50)},
        {S(-5,-50), S(-1,-30), S(2,-30), S(5,-30)},
        {S(-8,-50), S(-20,-30), S(4,-10), S(5,-10)},
        {S(-13,-50), S(-5,-30), S(2,-10), S(3,10)},
        {S(-13,-50), S(-6,-30), S(1,-10), S(2,10)},
        {S(-15,-50), S(-9,-30), S(-7,-10), S(-5,-10)},
        {S(-16,-50), S(-10,-30), S(-8,-30), S(-6,-30)},
        {S(-18,-50), S(-15,-50), S(-11,-50), S(-5,-50)},
    }, { // King
        {S(55,-50), S(50,-50), S(30,-50), S(30,-50)},
        {S(-10,-50), S(-10,-30), S(-10,-30), S(-10,-30)},
        {S(-20,-50), S(-20,-30), S(-20,-10), S(-20,-10)},
        {S(-30,-50), S(-30,-30), S(-30,-10), S(-30,10)},
        {S(-40,-50), S(-40,-30), S(-40,-10), S(-40,10)},
        {S(-50,-50), S(-50,-30), S(-50,-10), S(-50,-10)},
        {S(-60,-50), S(-60,-30), S(-60,-30), S(-60,-30)},
        {S(-70,-50), S(-70,-50), S(-70,-50), S(-70,-50)},
    }, 
};

int bishopPairBonus = S(20, 50);

void initEval() {
    // Add the material bonus to
    // each psqt square
    for (int i = 1; i < PIECE_TYPE_CNT; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 4; k++) {
                PSQTBonus[i][j][k] += materialBonus[i];
            }
        }
    }
}

int PSQTEval(Pos board) {
    int result = 0;

    int sq;

    while (board.sides[WHITE]) {
        sq = poplsb(&board.sides[WHITE]);
        result += PSQTBonus[board.pieceList[sq]][rank(sq)][(file(sq)>3 ? 7-file(sq) : file(sq))];
    }
    
    while (board.sides[BLACK]) {
        sq = poplsb(&board.sides[BLACK]);
        result -= PSQTBonus[pieceType(board.pieceList[sq])][7-rank(sq)][(file(sq)>3 ? 7-file(sq) : file(sq))];
    }

    return result;
}

int phase(Pos *board) {
    int p = 24;
    p -= popcnt(board->pieces[KNIGHT] | board->pieces[BISHOP]);
    p -= popcnt(board->pieces[ROOK]) * 2;
    p -= popcnt(board->pieces[QUEEN]) * 4;
    return (p * 256 + 12)/12;
}

// Evaluate a board position
int evaluate(Pos *board) {
    
    // Return value

    int result = 0;

    result += PSQTEval(*board);

    int p = phase(board);
    int mg = mgS(result);
    int eg = egS(result);
    result = ((mg * (256-p)) + (eg * p))/256;

    return (board->turn ? -result : result);
}
