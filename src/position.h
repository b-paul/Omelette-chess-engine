#pragma once

#include "attacks.h"
#include "bitboards.h"
#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

#include <stdbool.h>

void initZobrist();
void initPosition();
void resetBoard(Pos* board);
void printBoard(Pos board);
Bitboard sliderBlockers(Pos board, Square square);
int isDrawn(Pos *board, int height);
int makeMove(Pos* board, Move *move);
void undoMove(Pos* board, Move *move, Undo *undo);
Undo makeNullMove(Pos *board);
void undoNullMove(Pos *board, Undo undo);

Bitboard castlePath[CASTLE_CNT];
int castleBitMasks[SQ_CNT];

struct Pos {

    // Array showing the board and all of its pieces
    // Used for finding a piece at a square
    Piece pieceList[SQ_CNT];

    // Array of bitboards showing all pieces on the board
    // Categoriesed by piece type (not by side)
    Bitboard pieces[PIECE_TYPE_CNT];

    // Array of bitboards showing each sides pieces on the board
    Bitboard sides[CL_CNT];

    CastlePerms castlePerms;
    Colour turn;
    Square enPas;
    int fiftyMoveRule;

    int plyLength;
    Key history[357];

    // Zobrist hash key of the position
    Key hash;

    Score psqtScore;
};

struct Undo {
    Square lastEnPas;
    Key lastHash;
    CastlePerms lastCastle;
    int lastFiftyRule;
    Score lastPSQT;
};

// Is check
// side is the side to check
static inline Bitboard squareAttackers(Pos *board, Square sq, Colour side) {
    if (sq > 63) {
        printBoard(*board);
        printBitBoard(board->pieces[KING]);
        printBitBoard(board->sides[side]);
    }
    Bitboard them = board->sides[!side];
    Bitboard occ = board->sides[WHITE] | board->sides[BLACK];

    Bitboard theirPawns = board->pieces[PAWN] & them;
    Bitboard theirKnights = board->pieces[KNIGHT] & them;
    Bitboard theirBishops = board->pieces[BISHOP] & them;
    Bitboard theirRooks = board->pieces[ROOK] & them;
    Bitboard theirQueens = board->pieces[QUEEN] & them;
    Bitboard theirKing = board->pieces[KING] & them;

    Bitboard pawnAttackers = getPawnAttacks(sq, side) & theirPawns;
    Bitboard knightAttackers = getKnightAttacks(sq) & theirKnights;

    Bitboard diagAttacks = getBishopAttacks(sq, occ);
    Bitboard diagAttackers = (diagAttacks & theirBishops) | (diagAttacks & theirQueens);

    Bitboard horizontalAttacks = getRookAttacks(sq, occ);
    Bitboard horizontalAttackers = (horizontalAttacks & theirRooks) | (horizontalAttacks & theirQueens);

    Bitboard kingAttackers = getKingAttacks(sq) & theirKing;

    return pawnAttackers | knightAttackers | diagAttackers | horizontalAttackers | kingAttackers;
}

static inline bool hasNonPawnMaterial(Pos *board) {
    return (board->pieces[KNIGHT] ||
            board->pieces[BISHOP] ||
            board->pieces[ROOK] ||
            board->pieces[QUEEN]) ? 1 : 0;
}

static inline Square moveFrom(Move *move) {
    return move->value & 63;
}

static inline Square moveTo(Move *move) {
    return (move->value >> 6) & 63;
}

static inline PieceType pieceType(int piece) {
    // first 4 bits
    return (piece & 7);// - 1;
}

static inline MoveFlag moveType(Move *move) {
    return move->value & 0x18000;
}

static inline PieceType promotePiece(Move *move) {
    return (move->value & 0x7000) >> 12;
}

static inline bool castlePathAttacked(Pos *board, Bitboard castlePath) {
    while (castlePath) {
        if (squareAttackers(board, poplsb(&castlePath), board->turn)) {
            return 0;
        }
    }
    return 1;
}

static inline bool moveIsTactical(Move *move, Pos *board) {
    int to = moveTo(move);
    return board->pieceList[to] != NONE ||
           moveType(move) == ENPAS ||
           promotePiece(move);
}

static inline bool moveIsPseudolegal(Move *move, Pos *board) {
    if (move->value == NO_MOVE) return 0;

    int from = moveFrom(move);
    int to = moveTo(move);
    int piece = board->pieceList[to];

    if (piece == NONE) return 0;

    if (!(board->sides[board->turn] & (1ULL << from)) || 
        (board->sides[board->turn] & (1ULL << to)))
        return 0;

    if (moveType(move) == CASTLE) {
        int castle;
        switch (to) {
            case C1:
                castle = CAN_WHITE_QUEEN;
                break;
            case G1:
                castle = CAN_WHITE_KING;
                break;
            case C8:
                castle = CAN_BLACK_QUEEN;
                break;
            case G8:
                castle = CAN_BLACK_KING;
                break;
            default:
                assert(!CASTLE);
                return 0;
        }
        if (!(board->castlePerms & castle)) return 0;

        Bitboard occ = board->sides[WHITE] & board->sides[BLACK];

        Bitboard blockingPath = castlePath[castle];

        return !(occ & blockingPath) &&
            castlePathAttacked(board, blockingPath);
    }

    if (pieceType(piece) == PAWN) {
        if (moveType(move) == ENPAS) {
            return board->enPas == to &&
                getPawnAttacks(from, board->turn); 
        }

        if (moveType(move) == PROMOTE) {
            Bitboard rank8th = (board->turn ? Rank8 : Rank1);
            return (1ULL << to) & rank8th;
        }

        // Doublepush
         if (abs(to-from) == 16) {
             Bitboard rank4th = (board->turn ? Rank5 : Rank4);
             return (1ULL << to) & rank4th && board->pieceList[to] == NONE;
        }

        return (abs(to-from) == 8) ? true : (1ULL << to) & getPawnAttacks(from, board->turn);
    } else {
        Bitboard legalAttack;
        Bitboard occ = board->sides[WHITE] | board->sides[BLACK];
        switch (pieceType(piece)) {
            case KNIGHT:
                legalAttack = (1ULL << to) & getKnightAttacks(to);
                break;
            case BISHOP:
                legalAttack = (1ULL << to) & getBishopAttacks(to, occ);
                break;
            case ROOK:
                legalAttack = (1ULL << to) & getRookAttacks(to, occ);
                break;
            case QUEEN:
                legalAttack = (1ULL << to) & getQueenAttacks(to, occ);
                break;
            case KING:
                legalAttack = (1ULL << to) & getKingAttacks(to);
                break;
            default:
                printf("Invalid piece in moveIsPseudoLegal");
                assert(0);
                break;
        }
        return !moveType(move) && legalAttack;
    }
}
static inline void placePSQT(Pos *board, Piece piece, Square sq) {
    board->psqtScore += PSQT[piece][sq];
}
static inline void removePSQT(Pos *board, Piece piece, Square sq) {
    board->psqtScore -= PSQT[piece][sq];
}
