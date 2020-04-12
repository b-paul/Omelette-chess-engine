#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "types.h"

int MVVLVAValues[PIECE_TYPE_CNT] = {                                                                                                                        0,
    100, 200, 300, 400, 500, 600
};

void popMove(MoveList *moves, int index) {
    moves->moves[index] = moves->moves[--moves->count];
}

void initMovePicker(MovePicker *mp, Move ttMove) {
    mp->stage = TABLES_STAGE;

    mp->noisy.count = 0;
    mp->noisyIndex = 0;

    mp->quiet.count = 0;
    mp->quietIndex = 0;

    mp->ttMove = ttMove;
}

int nextMoveIndex(MoveList moves, int index) {
    int bestIndex = index;

    for (index++; index < moves.count; index++) {
        if (moves.moves[index].score > moves.moves[bestIndex].score) {
            bestIndex = index;
        }
    }

    return bestIndex;
}

void evalNoisy(MoveList *moves, Pos board) {
    // MVV-LVA is used
    int from,to,fPiece,tPiece;
    for (int i = 0; i < moves->count; i++) {
        from = moves->moves[i].value & 63;
        to = (moves->moves[i].value >> 6) & 63;
        fPiece = pieceType(board.pieceList[from]);
        tPiece = pieceType(board.pieceList[to]);
        moves->moves[i].score = MVVLVAValues[tPiece] - fPiece;
        
        // Since enpas moves dont have their capture on the
        // to square we have to set the score separately
        if (moveType(moves->moves[i]) == ENPAS)
            moves->moves[i].score = MVVLVAValues[PAWN] - PAWN;

        // Add a bonus for queen promotions too;
        if (promotePiece(moves->moves[i]) == QUEEN) {
            moves->moves[i].score += MVVLVAValues[QUEEN];
        }

    }
}

Move selectNextMove(MovePicker *mp, Pos *board, int onlyNoisy) {
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
            evalNoisy(&mp->noisy, *board);

            mp->noisyLeft = mp->noisy.count;

            mp->stage = PICK_WIN_NOISY;

        case PICK_WIN_NOISY:

            if (mp->noisyLeft) {
                mp->noisyLeft--;
                best = nextMoveIndex(mp->noisy, mp->noisyIndex);
                bestMove = mp->noisy.moves[best];
                if (bestMove.score >= 0) {
                    mp->noisyIndex++;
                    popMove(&mp->noisy, best);
                    if (bestMove.value == mp->ttMove.value)
                        return selectNextMove(mp, board, onlyNoisy);
                    return bestMove;
                }
                

            }
            if (onlyNoisy) {
                mp->stage = PICK_LOSE_NOISY;
                return selectNextMove(mp, board, onlyNoisy);
            }

            mp->stage = GEN_QUIETS; // change this to killers later

        case GEN_QUIETS:

            genMoves(&mp->quiet, board, QUIET);

            mp->quietLeft = mp->quiet.count;

            mp->stage = PICK_QUIETS;

        case PICK_QUIETS:

            if (mp->quietLeft) {
                mp->quietLeft--;
                best = nextMoveIndex(mp->quiet, mp->quietIndex);
                bestMove = mp->quiet.moves[best];
                mp->quietIndex++;
                popMove(&mp->quiet, best);
                if (bestMove.value == mp->ttMove.value)
                    return selectNextMove(mp, board, onlyNoisy);
                return bestMove;
            }

            mp->stage = PICK_LOSE_NOISY;
            
        case PICK_LOSE_NOISY:

            if (mp->noisyLeft) {
                mp->noisyLeft--;
                best = nextMoveIndex(mp->noisy, mp->noisyIndex);
                bestMove = mp->noisy.moves[best];
                mp->noisyIndex++;

                popMove(&mp->noisy, best);

                if (bestMove.value == mp->ttMove.value)
                    return selectNextMove(mp, board, onlyNoisy);
                return bestMove;

            }

        default:
            bestMove.value = NO_MOVE;
    }
    return bestMove;
}
