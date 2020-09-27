#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>

Key zobristPieces[SQ_CNT][PIECE_CNT];
Key zobristEnPas[8];
Key zobristCastle[15];
Key zobristTurn;

Key randBB() {
    static Key seed = 23409839408ULL;

    seed ^= seed >> 35;
    seed ^= seed << 12;
    seed ^= seed >> 53;

    return seed * 7163392348326487ull;
}

void initZobrist() {
    int i;
    int j;
    for (i = 0; i < SQ_CNT; i++) {
        for (j = 0; j < PIECE_CNT; j++) {
            zobristPieces[i][j] = randBB();
        }
    }

    for (i = 0; i < 8; i++) {
        zobristEnPas[i] = randBB();
    }

    for (i = 0; i < 15; i++) {
        // we dont use most of these but whatever
        zobristCastle[i] = randBB();
    }

    zobristTurn = randBB();
}

void initPosition() {
    castlePath[WHITE_KING_OCC] = 0x60;
    castlePath[WHITE_KING_ATTACK] = 0x70;
    castlePath[WHITE_QUEEN_OCC] = 0xE;
    castlePath[WHITE_QUEEN_ATTACK] = 0x1C;
    castlePath[BLACK_KING_OCC] = 0x6000000000000000;
    castlePath[BLACK_KING_ATTACK] = 0x7000000000000000;
    castlePath[BLACK_QUEEN_OCC] = 0xE00000000000000;
    castlePath[BLACK_QUEEN_ATTACK] = 0x1C00000000000000;

    castleBitMasks[A1] = CAN_WHITE_QUEEN;
    castleBitMasks[E1] = CAN_WHITE;
    castleBitMasks[H1] = CAN_WHITE_KING;
    castleBitMasks[A8] = CAN_BLACK_QUEEN;
    castleBitMasks[E8] = CAN_BLACK;
    castleBitMasks[H8] = CAN_BLACK_KING;

}

void resetBoard(Pos* board) {

    int i;

    // Loop through each square and set to none
    for (i = 0; i < SQ_CNT; i++) {
        board->pieceList[i] = NONE;
    }

    // Empty each bitboard
    for (i = 0; i < PIECE_TYPE_CNT; i++) {
        board->pieces[i] = 0ULL;
    }

    for (i = 0; i < CL_CNT; i++) {
        board->sides[i] = 0ULL;
    }

    board->turn = WHITE;

    board->castlePerms = 0;
    board->enPas = NO_SQ;
    board->fiftyMoveRule = 0;
    board->plyLength = 0;
    board->psqtScore = 0;
}

void printBoard(Pos board) {
    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file < FILE_CNT; file++) {
            // For each square, print based on the piece
            switch(board.pieceList[sq(rank, file)]) {
                case wP: printf("P"); break;
                case wN: printf("N"); break;
                case wB: printf("B"); break;
                case wR: printf("R"); break;
                case wQ: printf("Q"); break;
                case wK: printf("K"); break;
                case bP: printf("p"); break;
                case bN: printf("n"); break;
                case bB: printf("b"); break;
                case bR: printf("r"); break;
                case bQ: printf("q"); break;
                case bK: printf("k"); break;
                default: printf("-"); break;
            }
        }
        printf("\n");
    }
    printf("\n\n");
}



Bitboard sliderBlockers(Pos board, const int square) {
        Bitboard result = 0ull;

        Bitboard sliders = ((bPseudoAttacks[square] & (board.pieces[BISHOP] | board.pieces[QUEEN])) |
                                    (rPseudoAttacks[square] & (board.pieces[ROOK] | board.pieces[QUEEN]))) & board.sides[!board.turn];
        Bitboard occ = (board.pieces[WHITE] | board.pieces[BLACK]) ^ sliders;
        while (sliders) {
                int sliderSquare = poplsb(&sliders);
                Bitboard betweenSqs = betweenBB(square, sliderSquare) & occ; 
                if (betweenSqs && multipleBits(betweenSqs)) {
                        result |= betweenSqs;
                }
        }
        return result;
}

int isFiftyMoveDraw(Pos *board) {
    // Fifty moves == 100 plys
    return board->fiftyMoveRule >= 100;
    // Should check for not mate as well
}

int isRepetitionDraw(Pos *board, const int height) {
    int repeats = 0; 

    for (int i = board->plyLength-2; i >= 0; i-=2) {
        if (i < board->plyLength - board->fiftyMoveRule) break;

        if (board->history[i] == board->hash &&
           (i > board->plyLength - height || ++repeats == 2)) {
            return 1;

        }
    }
    return 0;
}

int isMaterialDraw(Pos *board) {
    return !(board->pieces[PAWN] | board->pieces[ROOK] | board->pieces[QUEEN]) &&
           (!multipleBits(board->sides[WHITE]) || !multipleBits(board->sides[BLACK])) &&
           (!multipleBits(board->pieces[BISHOP] | board->pieces[KNIGHT]) ||
           (!board->pieces[BISHOP] && popcnt(board->pieces[KNIGHT]) <= 2));
}

int isDrawn(Pos *board, const int height) {
    return isFiftyMoveDraw(board) ||
           isRepetitionDraw(board, height) ||
           isMaterialDraw(board);
}



int makeMove(Pos* board, Move *move) {
    if (move->value == NO_MOVE) return 0;

    board->fiftyMoveRule++;

    board->history[board->plyLength++] = board->hash;

    board->didNullMove = FALSE;

    // First and second 6 bits
    int from = moveFrom(move);
    int to = moveTo(move);
    Bitboard tobb = 1ULL << to;
    Bitboard frombb = 1ULL << from;
    Bitboard toFrom = frombb | tobb;

    assert(validSquare(from));
    assert(validSquare(to));
    assert(board->pieceList[from] != NONE);

    assert(board->pieces[KING] & board->sides[board->turn]);

    int piece = board->pieceList[from];

    int pushDir = (board->turn) ? -8 : 8;

    if (moveType(move) == CASTLE) {

        int rookTo = sq(rank(to), (to > from) ? 5 : 3);
        int rookFrom = sq(rank(to), (to > from) ? 7 : 0);
        int rookPiece = (board->turn) ? bR : wR;
        Bitboard rToFrom = (1ULL << rookTo) | (1ULL << rookFrom);

        board->pieces[ROOK] ^= rToFrom;
        board->sides[board->turn] ^= rToFrom;
        board->pieceList[rookFrom] = NONE;
        board->pieceList[rookTo] = rookPiece;
        board->hash ^= zobristPieces[rookFrom][board->pieceList[rookFrom]];
        board->hash ^= zobristPieces[rookTo][board->pieceList[rookTo]];
        board->psqtScore -= PSQT[rookPiece][rookFrom];
        board->psqtScore += PSQT[rookPiece][rookTo];

    } else if (moveType(move) == PROMOTE) {

        assert(pieceType(piece) == PAWN);

        int promote = promotePiece(move);

        board->hash ^= zobristPieces[from][piece];
        board->psqtScore -= PSQT[piece][from];

        piece += promote - 1;

        board->pieces[PAWN] ^= frombb;
        board->pieces[promote] ^= frombb;
        board->hash ^= zobristPieces[from][piece];
        board->psqtScore += PSQT[piece][from];
    }

    if (board->castlePerms && (castleBitMasks[to] | castleBitMasks[from])) { 
        int bit = castleBitMasks[to] | castleBitMasks[from];
        board->hash ^= zobristCastle[board->castlePerms & bit];
        board->castlePerms &= ~bit;
    }
        

    // Captures
    
    if (moveType(move) == ENPAS) {

        int capSq = to - pushDir;

        assert(pieceType(piece) == PAWN);
        // The captured piece is a pawn
        assert((board->pieceList[capSq] & 7) == PAWN);

        Bitboard cap = 1ULL << capSq;
        board->hash ^= zobristPieces[capSq][board->pieceList[capSq]];
        board->pieces[PAWN] ^= cap;
        board->sides[!board->turn] ^= cap;
        move->lastCapture = board->pieceList[capSq]; 
        board->pieceList[capSq] = NONE;
        board->psqtScore -= PSQT[move->lastCapture][capSq];
    } else if (board->pieceList[to] != NONE) {

        // Cant capture our own piece
        assert(((board->pieceList[to] >> 3) & 1) != board->turn);

        Bitboard cap = 1ULL << to;

        move->lastCapture = board->pieceList[to];
        
        assert(pieceType(move->lastCapture) != KING);

        board->pieces[pieceType(move->lastCapture)] ^= cap;
        board->sides[!board->turn] ^= cap;
        board->hash ^= zobristPieces[to][move->lastCapture];
        board->psqtScore -= PSQT[move->lastCapture][to];

        board->fiftyMoveRule = 0;

   } else move->lastCapture = NONE;

    if (board->enPas != NO_SQ) {
        board->hash ^= zobristEnPas[file(board->enPas)];
        board->enPas = NO_SQ;
    }

    // Move the piece

    board->pieces[pieceType(piece)] ^= toFrom;
    board->sides[board->turn] ^= toFrom;
    board->pieceList[from] = NONE;
    board->pieceList[to] = piece;
    board->hash ^= zobristPieces[from][piece];
    board->hash ^= zobristPieces[to][piece];
    board->psqtScore -= PSQT[piece][from];
    board->psqtScore += PSQT[piece][to];

    board->turn = !board->turn;
    board->hash ^= zobristTurn;

    // Update enPas

    if (pieceType(piece) == PAWN) {
        board->fiftyMoveRule = 0;
        if (abs(to-from) == 16) {
            board->enPas = to - pushDir;
            board->hash ^= zobristEnPas[file(board->enPas)];
        } 
    }

    assert(board->pieces[KING] & board->sides[!board->turn]);

    if (squareAttackers(board, getlsb(board->pieces[KING] & board->sides[!board->turn]), !board->turn)) {
        return 0;
    }
    return 1;
}

void undoMove(Pos* board, Move *move, Undo *undo) {

    board->enPas = undo->lastEnPas;
    board->hash = undo->lastHash;
    board->castlePerms = undo->lastCastle;
    board->fiftyMoveRule = undo->lastFiftyRule;
    board->psqtScore = undo->lastPSQT;
    board->didNullMove = undo->lastDidNullMove;

    board->plyLength--;

    // First and secont 6 bits
    int from = moveFrom(move);
    int to = moveTo(move);
    Bitboard tobb = 1ULL << to;
    Bitboard toFrom = (1ULL << from) | tobb;

    int piece = board->pieceList[to];

    board->turn = !board->turn;

    if (moveType(move) == CASTLE) {

        int rookTo = sq(rank(to), (to > from) ? 5 : 3);
        int rookFrom = sq(rank(to), (to > from) ? 7 : 0);
        int rookPiece = (board->turn) ? bR : wR;
        Bitboard rToFrom = (1ULL << rookTo) | (1ULL << rookFrom);

        board->pieces[ROOK] ^= rToFrom;
        board->sides[board->turn] ^= rToFrom;
        board->pieceList[rookFrom] = rookPiece;
        board->pieceList[rookTo] = NONE;

    } else if (moveType(move) == PROMOTE) {

        assert(pieceType(piece) != PAWN);

        int promote = promotePiece(move);
        
        piece -= (promote - 1);

        board->pieces[PAWN] ^= tobb;
        board->pieces[promote] ^= tobb;
    }

    board->pieces[pieceType(piece)] ^= toFrom;
    board->sides[board->turn] ^= toFrom;
    board->pieceList[from] = piece;

    if (moveType(move) == ENPAS) {
        
        assert(pieceType(move->lastCapture) == PAWN);

        int pushDir = (board->turn) ? 8 : -8;
        Bitboard epBB = shift(tobb, pushDir); 
        int capSq = to + pushDir;
        board->pieceList[capSq] = move->lastCapture;
        board->pieces[PAWN] ^= epBB;
        board->sides[!board->turn] ^= epBB;
        board->pieceList[to] = NONE;

    } else if (move->lastCapture) {

        board->pieceList[to] = move->lastCapture;
        board->pieces[pieceType(move->lastCapture)] ^= tobb;
        board->sides[!board->turn] ^= tobb;
    
    } else {
        board->pieceList[to] = NONE;
    }
}

Undo makeNullMove(Pos *board) {
    Undo undo;

    undo.lastEnPas = board->enPas;
    undo.lastHash = board->hash;
    undo.lastFiftyRule = board->fiftyMoveRule;

    board->fiftyMoveRule++;
    board->history[board->plyLength++] = board->hash;
    board->didNullMove = TRUE;

    board->turn = !board->turn;
    board->hash ^= zobristTurn;

    if (board->enPas != NO_SQ) {
        board->hash ^= zobristEnPas[file(board->enPas)];
        board->enPas = NO_SQ;
    }

    return undo;
}

void undoNullMove(Pos *board, Undo undo) {
    board->enPas = undo.lastEnPas;
    board->hash = undo.lastHash;
    board->fiftyMoveRule = undo.lastFiftyRule;
    board->didNullMove = FALSE;

    board->plyLength--;

    board->turn = !board->turn;
}
