#include "attacks.h"
#include "bitboards.h"
#include "movegen.h"
#include "position.h"
#include "types.h"


// Macro to allow moves to be stored in a single integer
#define makeMove(t, f, promotion, flag) (flag + (promotion << 12) + (t << 6) + f)



void makePromotion(MoveList* moves, int to, int from, int type) {

    if (type != QUIET) {

        // Only gen queen promotions in non-quiet moves
        moves->moves[moves->count].value = makeMove(to, from, QUEEN, PROMOTE);
        moves->count++;

    }
    if (type != NOISY) {
        
        // Gen underpromotions in non-noisy moves
        moves->moves[moves->count].value = makeMove(to, from, ROOK, PROMOTE);
        moves->count++;
        moves->moves[moves->count].value = makeMove(to, from, BISHOP, PROMOTE);
        moves->count++;
        moves->moves[moves->count].value = makeMove(to, from, KNIGHT, PROMOTE);
        moves->count++;
        
    }
}



void genPawnMoves(MoveList* moves, Pos board, int type) {
    Bitboard occ = board.sides[WHITE] | board.sides[BLACK];
    Bitboard empty = ~occ;

    int sq;
    int pushDir = (board.turn) ? -8 : 8;
    int capDir1 = (board.turn) ? -7 : 7;
    int capDir2 = (board.turn) ? -9 : 9;

    Bitboard rank4th = (board.turn ? Rank5 : Rank4);
    Bitboard rank5th = (board.turn ? Rank4 : Rank5);
    Bitboard rank7th = (board.turn ? Rank2 : Rank7);

    Bitboard ourPawns = board.pieces[PAWN] & board.sides[board.turn];

    Bitboard them = board.sides[!board.turn];

    Bitboard promote = ourPawns & rank7th;
    Bitboard noPromote = ourPawns & ~rank7th;

    if (type != NOISY) {
        Bitboard p1 = shift(noPromote, pushDir) & empty;
        Bitboard p2 = shift(p1, pushDir) & rank4th & empty;


        while (p1) {
            sq = poplsb(&p1);
            moves->moves[moves->count].value = makeMove(sq, (sq - pushDir), 0, NORMAL);
            moves->count++;
        }

        while (p2) {
            sq = poplsb(&p2);
            moves->moves[moves->count].value = makeMove(sq, (sq - pushDir - pushDir), 0, NORMAL);
            moves->count++;
        }
    }

    if (promote) {
        Bitboard p1 = shift(promote, pushDir) & empty;
        Bitboard p2 = shift(promote, capDir1) & them;
        Bitboard p3 = shift(promote, capDir2) & them;

        while (p1) {
            sq = poplsb(&p1);
            makePromotion(moves, sq, (sq - pushDir), type);
        }

        while (p2) {
            sq = poplsb(&p2);
            makePromotion(moves, sq, (sq - capDir1), type);
        }

        while (p3) {
            sq = poplsb(&p3);
            makePromotion(moves, sq, (sq - capDir2), type);
        }
    }


    if (type != QUIET) {
        Bitboard p1 = shift(noPromote, capDir1) & them;
        Bitboard p2 = shift(noPromote, capDir2) & them;

        while (p1) {
            sq = poplsb(&p1);
            moves->moves[moves->count].value = makeMove(sq, (sq - capDir1), 0, NORMAL);
            moves->count++;
        }

        while (p2) {
            sq = poplsb(&p2);
            moves->moves[moves->count].value = makeMove(sq, (sq - capDir2), 0, NORMAL);
            moves->count++;
        }

        if (board.enPas != NO_SQ) {
            p1 = noPromote & getPawnAttacks(board.enPas, !board.turn) & rank5th;
            while (p1) {
                moves->moves[moves->count].value = makeMove(board.enPas, poplsb(&p1), 0, ENPAS);
                moves->count++;
            }
        }
    }
}

void genMoves(MoveList* moves, Pos *board, int type) {
    board->lastEnPas = board->enPas;
    board->lastHash = board->hash;
    board->lastCastle = board->castlePerms;
    board->lastFiftyRule = board->fiftyMoveRule;

    genPawnMoves(moves, *board, type);

    // return;

    Bitboard ourKnights = board->pieces[KNIGHT] & board->sides[board->turn];
    Bitboard ourBishops = board->pieces[BISHOP] & board->sides[board->turn];
    Bitboard ourRooks = board->pieces[ROOK] & board->sides[board->turn];
    Bitboard ourQueens = board->pieces[QUEEN] & board->sides[board->turn];
    Bitboard ourKing = board->pieces[KING] & board->sides[board->turn];

    Bitboard occ = board->sides[WHITE] | board->sides[BLACK];

    Bitboard targets = (type == ALLMOVES) ? ~board->sides[board->turn] :
                                         (type == NOISY)        ? board->sides[!board->turn] :
                                                                                    ~occ;

    // return;

    int from;

    Bitboard attacks;

    // Knights

    while (ourKnights) {
        from = poplsb(&ourKnights);
        attacks = getKnightAttacks(from) & targets;
        while (attacks) {
            moves->moves[moves->count].value = makeMove(poplsb(&attacks), from, 0, NORMAL);
            moves->count++;
        }
    }

    // Bishops

    while (ourBishops) {
        from = poplsb(&ourBishops);
        attacks = getBishopAttacks(from, occ) & targets;
        while (attacks) {
            moves->moves[moves->count].value = makeMove(poplsb(&attacks), from, 0, NORMAL);
            moves->count++;
        }
    }

    // Rooks

    while (ourRooks) {
        from = poplsb(&ourRooks);
        attacks = getRookAttacks(from, occ) & targets;
        while (attacks) {
            moves->moves[moves->count].value = makeMove(poplsb(&attacks), from, 0, NORMAL);
            moves->count++;
        }
    }

    // Queen/s

    while (ourQueens) {
        from = poplsb(&ourQueens);
        attacks = getQueenAttacks(from, occ) & targets;
        while (attacks) {
            moves->moves[moves->count].value = makeMove(poplsb(&attacks), from, 0, NORMAL);
            moves->count++;
        }
    }

    // King

    from = poplsb(&ourKing);
    attacks = getKingAttacks(from) & targets;
    while (attacks) {
        moves->moves[moves->count].value = makeMove(poplsb(&attacks), from, 0, NORMAL);
        moves->count++;
    }

    // Castling

    if (type != NOISY) {
        if (board->turn) { // Black castling
            if ((board->castlePerms & CAN_BLACK_KING) && !(occ & castlePath[BLACK_KING_OCC]) && castlePathAttacked(*board, castlePath[BLACK_KING_ATTACK])) { // Kingside
                moves->moves[moves->count].value = makeMove(G8, E8, 0, CASTLE);
                moves->count++;
            } 
            if ((board->castlePerms & CAN_BLACK_QUEEN) && !(occ & castlePath[BLACK_QUEEN_OCC]) && castlePathAttacked(*board, castlePath[BLACK_QUEEN_ATTACK])) { // Queenside
                moves->moves[moves->count].value = makeMove(C8, E8, 0, CASTLE);
                moves->count++;
            }
        } else { // White castling
            if ((board->castlePerms & CAN_WHITE_KING) && !(occ & castlePath[WHITE_KING_OCC]) && castlePathAttacked(*board, castlePath[WHITE_KING_ATTACK])) { // Kingside
                moves->moves[moves->count].value = makeMove(G1, E1, 0, CASTLE);
                moves->count++;
            } 
            if ((board->castlePerms & CAN_WHITE_QUEEN) && !(occ & castlePath[WHITE_QUEEN_OCC]) && castlePathAttacked(*board, castlePath[WHITE_QUEEN_ATTACK])) { // Queenside
                moves->moves[moves->count].value = makeMove(C1, E1, 0, CASTLE);
                moves->count++;
            }
        }
    }
}
