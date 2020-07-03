#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "types.h"

#include <stdbool.h>

// Zero out history table
void initHistoryTable(HistoryTable *table) {
    for (int i = 0; i < SQ_CNT; i++) {
        for (int j = 0; j < SQ_CNT; j++) {
            table->historyScores[WHITE][i][j] = 0;
            table->historyScores[BLACK][i][j] = 0;
        }
    }

    for (int i = 0; i < 2048; i++) {
        table->killers[i][0].value = 0;
        table->killers[i][0].score = 0;
        table->killers[i][1].value = 0;
        table->killers[i][1].score = 0;
    }
}

// If there is a beta cutoff, store the move into
// a table
void updateHistoryScores(HistoryTable *table, Pos *board, Move *move, int depth, int height) {
    int bonus = SQUARED(depth);
    table->historyScores[board->turn][moveFrom(move)][moveTo(move)] += bonus;

    if (table->killers[height][0].value != move->value) {
        table->killers[height][1] = table->killers[height][1];
        table->killers[height][0] = *move;
    }
}

// Scores used for MVV-LVA
// MVV-LVA is used for evaluating
// the strength of a capture
int MVVLVAValues[PIECE_TYPE_CNT] = {
    0,
    // Pieces start at index 1
    100, 200, 300, 400, 500, 600
};

// Remove the move at index
void popMove(MoveList *moves, int index) {
    moves->moves[index] = moves->moves[--moves->count];
}

// This zeroes out some values and initializes
// things like the ttMove
void initMovePicker(MovePicker *mp, Pos *board, Move *ttMove, int height) {

    // Init the undo
    mp->undo.lastEnPas = board->enPas;
    mp->undo.lastHash = board->hash;
    mp->undo.lastCastle = board->castlePerms;
    mp->undo.lastFiftyRule = board->fiftyMoveRule;
    mp->undo.lastPSQT = board->psqtScore;

    mp->stage = TABLES_STAGE;

    mp->height = height;

    mp->noisy.count = 0;

    mp->quiet.count = 0;

    Move noMove;
    noMove.value = NO_MOVE;
    mp->ttMove = (moveIsPseudolegal(ttMove, board) ? *ttMove : noMove);
}

// Find the next best move in the list based on the
// score value of the moves
int nextMoveIndex(MoveList *moves, int index) {
    int bestIndex = index;

    for (index++; index < moves->count; index++) {
        if (moves->moves[index].score > moves->moves[bestIndex].score) {
            bestIndex = index;
        }
    }

    return bestIndex;
}

// Use MVV-LVA to guess how good a capture
// generally is, and store it into the score
// of the move
void evalNoisy(MoveList *moves, Pos *board) {
    // MVV-LVA is used
    Square from,to;
    Piece fPiece,tPiece;
    for (int i = 0; i < moves->count; i++) {
        from = moveFrom(&moves->moves[i]);
        to = moveTo(&moves->moves[i]);
        fPiece = pieceType(board->pieceList[from]);
        tPiece = pieceType(board->pieceList[to]);
        moves->moves[i].score = MVVLVAValues[tPiece] - fPiece;
        
        // Since enpas moves dont have their capture on the
        // to square we have to set the score separately
        if (moveType(&moves->moves[i]) == ENPAS)
            moves->moves[i].score = MVVLVAValues[PAWN] - PAWN;

        // Add a bonus for queen promotions too;
        if (promotePiece(&moves->moves[i]) == QUEEN) {
            moves->moves[i].score += MVVLVAValues[QUEEN];
        }

    }
}

// Lookup history scores and killer moves and 
// evaluate quiet moves based on these scores
void evalQuiet(MoveList *moves, HistoryTable *hTable, Pos *board, int height) {
    Piece from, to;
    for (int i = 0; i < moves->count; i++) {
        // If the move is a killer move
        // set the score to a big number;
        if (moves->moves[i].value == hTable->killers[height][0].value) {
            moves->moves[i].score = 1ull << 31;
        } else if (moves->moves[i].value == hTable->killers[height][1].value) {
            moves->moves[i].score = 1ull << 30;
        }
        from = moveFrom(&moves->moves[i]);
        to = moveTo(&moves->moves[i]);
        moves->moves[i].score = hTable->historyScores[board->turn][from][to];
    }
}

Move selectNextMove(MovePicker *mp, HistoryTable *hTable, Pos *board, bool onlyNoisy) {
    int best;
    Move bestMove;
    switch (mp->stage) {
        case TABLES_STAGE:
            mp->stage = GEN_NOISY;
            if (mp->ttMove.value != NO_MOVE) {
                bestMove = mp->ttMove;
                return bestMove;
            }
        case GEN_NOISY:

            genMoves(&mp->noisy, board, NOISY);
            evalNoisy(&mp->noisy, board);

            mp->noisyLeft = mp->noisy.count;

            mp->stage = PICK_WIN_NOISY;

        case PICK_WIN_NOISY:

            if (mp->noisyLeft) {
                mp->noisyLeft--;
                best = nextMoveIndex(&mp->noisy, 0);
                bestMove = mp->noisy.moves[best];
                if (bestMove.score >= 0) {
                    popMove(&mp->noisy, best);
                    if (bestMove.value == mp->ttMove.value)
                        return selectNextMove(mp, hTable, board, onlyNoisy);
                    return bestMove;
                }
                

            }
            if (onlyNoisy) {
                mp->stage = PICK_LOSE_NOISY;
                return selectNextMove(mp, hTable, board, onlyNoisy);
            }

            mp->stage = GEN_QUIETS;

        case GEN_QUIETS:

            genMoves(&mp->quiet, board, QUIET);
            evalQuiet(&mp->quiet, hTable, board, mp->height);

            mp->quietLeft = mp->quiet.count;

            mp->stage = PICK_QUIETS;

        case PICK_QUIETS:

            if (mp->quietLeft) {
                mp->quietLeft--;
                best = nextMoveIndex(&mp->quiet, 0);
                bestMove = mp->quiet.moves[best];

                popMove(&mp->quiet, best);
                if (bestMove.value == mp->ttMove.value)
                    return selectNextMove(mp, hTable, board, onlyNoisy);
                return bestMove;

            }

            mp->stage = PICK_LOSE_NOISY;
            
        case PICK_LOSE_NOISY:

            if (mp->noisyLeft) {
                mp->noisyLeft--;
                best = nextMoveIndex(&mp->noisy, 0);
                bestMove = mp->noisy.moves[best];

                popMove(&mp->noisy, best);

                if (bestMove.value == mp->ttMove.value)
                    return selectNextMove(mp, hTable, board, onlyNoisy);
                return bestMove;

            }

        default:
            bestMove.value = NO_MOVE;
    }
    return bestMove;
}
