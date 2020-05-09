#pragma once

#include "attacks.h"
#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

void initZobrist();
void initPosition();
void resetBoard(Pos* board);
void printBoard(Pos board);
Bitboard sliderBlockers(Pos board, int square);
int isDrawn(Pos board, int height);
int makeMove(Pos* board, Move *move);
void undoMove(Pos* board, Move move, Undo undo);
Undo makeNullMove(Pos *board);
void undoNullMove(Pos *board, Undo undo);

Bitboard castlePath[CASTLE_CNT];
int castleBitMasks[SQ_CNT];

struct Pos {

    // Array showing the board and all of its pieces
    // Used for finding a piece at a square
    int pieceList[SQ_CNT];

    // Array of bitboards showing all pieces on the board
    // Categoriesed by piece type (not by side)
    Bitboard pieces[PIECE_TYPE_CNT];

    // Array of bitboards showing each sides pieces on the board
    Bitboard sides[CL_CNT];

    int castlePerms;
    int turn;
    int enPas;
    int fiftyMoveRule;

    int plyLength;
    Key history[2048];

    // Zobrist hash key of the position
    Key hash;
};

struct Undo {
    int lastEnPas;
    Key lastHash;
    int lastCastle;
    int lastFiftyRule;
};

// Is check
// side is the side to check
static inline Bitboard squareAttackers(Pos board, int sq, int side) {
    Bitboard them = board.sides[!side];
    Bitboard occ = board.sides[WHITE] | board.sides[BLACK];

    Bitboard theirPawns = board.pieces[PAWN] & them;
    Bitboard theirKnights = board.pieces[KNIGHT] & them;
    Bitboard theirBishops = board.pieces[BISHOP] & them;
    Bitboard theirRooks = board.pieces[ROOK] & them;
    Bitboard theirQueens = board.pieces[QUEEN] & them;
    Bitboard theirKing = board.pieces[KING] & them;

    Bitboard pawnAttackers = getPawnAttacks(sq, side) & theirPawns;
    Bitboard knightAttackers = getKnightAttacks(sq) & theirKnights;

    Bitboard diagAttacks = getBishopAttacks(sq, occ);
    Bitboard diagAttackers = (diagAttacks & theirBishops) | (diagAttacks & theirQueens);

    Bitboard horizontalAttacks = getRookAttacks(sq, occ);
    Bitboard horizontalAttackers = (horizontalAttacks & theirRooks) | (horizontalAttacks & theirQueens);

    Bitboard kingAttackers = getKingAttacks(sq) & theirKing;

    return pawnAttackers | knightAttackers | diagAttackers | horizontalAttackers | kingAttackers;
}

static inline int hasNonPawnMaterial(Pos board) {
    return (board.pieces[KNIGHT] ||
            board.pieces[BISHOP] ||
            board.pieces[ROOK] ||
            board.pieces[QUEEN]) ? 1 : 0;
}

static inline int moveFrom(Move move) {
    return move.value & 63;
}

static inline int moveTo(Move move) {
    return (move.value >> 6) & 63;
}

static inline int pieceType(int piece) {
    // first 4 bits
    return (piece & 7);// - 1;
}

static inline int moveType(Move move) {
    return move.value & 0x18000;
}

static inline int promotePiece(Move move) {
    return (move.value & 0x7000) >> 12;
}

static inline int castlePathAttacked(Pos board, Bitboard castlePath) {
    while (castlePath) {
        if (squareAttackers(board, poplsb(&castlePath), board.turn)) {
            return 0;
        }
    }
    return 1;
}

static inline int moveIsTactical(Move move, Pos board) {
    int to = moveTo(move);
    return board.pieceList[to] != NONE ||
           moveType(move) == ENPAS ||
           promotePiece(move);
}
