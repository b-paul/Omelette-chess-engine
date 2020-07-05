#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "types.h"

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

void updateHistoryScores(HistoryTable *table, Pos *board, Move *move, int depth, int height) {
    int bonus = SQUARED(depth);
    table->historyScores[board->turn][moveFrom(move)][moveTo(move)] += bonus;

    if (table->killers[height][0].value != move->value) {
        table->killers[height][1] = table->killers[height][1];
        table->killers[height][0] = *move;
    }
}

int MVVLVAValues[PIECE_TYPE_CNT] = {                                                                                                                        0,
    100, 200, 300, 400, 500, 600
};

void popMove(MoveList *moves, int index) {
    moves->moves[index] = moves->moves[--moves->count];
}

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

int nextMoveIndex(MoveList *moves, int index) {
    int bestIndex = index;

    for (index++; index < moves->count; index++) {
        if (moves->moves[index].score > moves->moves[bestIndex].score) {
            bestIndex = index;
        }
    }

    return bestIndex;
}

void evalNoisy(MoveList *moves, Pos *board) {
    // MVV-LVA is used
    int from,to,fPiece,tPiece;
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

void evalQuiet(MoveList *moves, HistoryTable *hTable, Pos *board, int height) {
    int from, to;
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

Move selectNextMove(MovePicker *mp, HistoryTable *hTable, Pos *board, int onlyNoisy) {
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
