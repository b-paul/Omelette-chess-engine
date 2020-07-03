#include "evaluate.h"
#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "threads.h"
#include "time.h"
#include "tt.h"
#include "types.h"
#include "uci.h"
#include "fathom/tbprobe.h"

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

volatile bool STOP_SEARCH = 0;

Score futilityMargin = 230;

unsigned long long perft(Pos *board, int depth, bool isRoot) {
    if (depth == 0) return 1;

    MoveList moves;
    moves.count = 0;
    genMoves(&moves, board, ALLMOVES);

    unsigned long long nodes = 0;

    MovePicker mp;
    Move ttMove;
    ttMove.value = NO_MOVE;

    initMovePicker(&mp, board, &ttMove, 0);

    for (int i = 0; i < moves.count; i++) {
        if (!makeMove(board, &moves.moves[i])) {
            undoMove(board, &moves.moves[i], &mp.undo);
            continue;
        }
        nodes += perft(board, depth - 1, 0);
        undoMove(board, &moves.moves[i], &mp.undo);
    }
    return nodes;
}

int reductionTable[64][64];

void initSearch() {
    for (Square i = 0; i < SQ_CNT; i++)
        for (Square j = 0; j < SQ_CNT; j++) {
            reductionTable[i][j] = 1 + (int)((log(i)*log(j))/10);
        }
}

Score qsearch(Pos *board, Score alpha, Score beta, int height, Thread *thread, PrincipalVariation *pv) {
    thread->nodes++;
    if (height > thread->seldepth) thread->seldepth = height;

    // If we do not have enough time left or
    // the gui has requested a stop, stop searching
    if (STOP_SEARCH || stopSearch(thread)) longjmp(thread->jumpEnv, 1);

    Score standPat = evaluate(board);

    // If our evaluation is greater than
    // beta, cutoff
    if (standPat >= beta)
        return standPat;

    // If our evaluation is greater than
    // alpha, set alpha to the eval
    if (standPat > alpha)
        alpha = standPat;

    Score score, bestScore = standPat;
    Move move;

    PrincipalVariation lastPv;
    lastPv.length = 0;
    pv->length = 0;

    Move ttMove;
    ttMove.value = NO_MOVE;

    MovePicker mp;

    initMovePicker(&mp, board, &ttMove, height);

    // Iterate over every move
    while ((move = selectNextMove(&mp, thread->hTable, board, 1)).value != NO_MOVE) {
        // If the move is illegal, undo the move
        if (!makeMove(board, &move)) {
            undoMove(board, &move, &mp.undo);
            continue;
        }

        // Search our new position
        score = -qsearch(board, -beta, -alpha, height+1, thread, &lastPv);

        // Undo after we have completed the search
        undoMove(board, &move, &mp.undo);

        // If we have raised the best score, set the best score
        if (score > bestScore) {
            bestScore = score;

            // If we have raised alpha, set alpha
            if (score > alpha) {
                alpha = score;
                
                // Write to the pv on an alpha raise
                pv->length = lastPv.length + 1;
                pv->pv[0] = move;
                memcpy(pv->pv+1, lastPv.pv, sizeof(Move) * lastPv.length);

                // Beta cutoff
                if (alpha >= beta) {
                    break;
                } 
            }
        }
    }

    return bestScore;
}

int alphaBeta(Pos *board, Score alpha, Score beta, int depth, int height, Thread *thread, PrincipalVariation *pv) {
    if (depth <= 0) {
        return qsearch(board, alpha, beta, height, thread, pv);
    }

    thread->nodes++;
    if (height > thread->seldepth) thread->seldepth = height;

    if (STOP_SEARCH || stopSearch(thread)) longjmp(thread->jumpEnv, 1);

    // check for draws
    if (isDrawn(board, height)) return 0;

    PrincipalVariation lastPv;
    lastPv.length = 0;
    pv->length = 0;

    Score score,bestScore=-INF;
    int movecnt = 0;
    Move move, bestMove;

    // This bool determines whether the search is
    // in a full window search or a zero window search
    // When it is in a zero window search, alpha
    // is 1 less than beta
    // This makes it so that if there is an increase
    // in alpha it will almost always also cause a beta cutoff
    bool PVNode = alpha != beta-1;

    bool RootNode = height == 0;

    Score eval = evaluate(board);

    bool isQuiet;
    int R;
    bool didLMR;

    bool inCheck = squareAttackers(board, getlsb(board->pieces[KING] & board->sides[board->turn]), board->turn) ? 1 : 0;
    depth += inCheck;

    MovePicker mp;

    ttEntry *hashEntry;
    bool isReplaced;
    Score ttEval;
    Move ttMove;

    // Probe TT
    hashEntry = probeTT(thread->tt, board->hash, &isReplaced);
    ttEval = isReplaced ? hashEntry->eval : 0;
    ttMove.value = isReplaced ? hashEntry->move.value : NO_MOVE;

    // Cutoff if we have searched this position
    // to a greater or equal depth
    if (!PVNode &&
        isReplaced &&
        hashEntry->depth >= depth) {
        return ttEval;
    }

    // Syzygy tablebase probing
    // The probeSyzygyWDL function only
    // probes the tablebase if the last move
    // was a capture since that is the
    // only time the amount of pieces on the board
    // will decrease
    unsigned entry;
    if (!RootNode &&
        (entry = probeSyzygyWDL(board)) != TB_RESULT_FAILED) {

        thread->tbHits++;

        score = entry == TB_LOSS ? -WDL_WIN + height :
                    entry == TB_WIN ? WDL_WIN - height :
                    0;

        Bound bound = entry == TB_LOSS ? UPPER :
                    entry == TB_WIN ? LOWER :
                    EXACT;

        if  (bound == EXACT ||
            (bound == LOWER && score >= beta) ||
            (bound == UPPER && score <= alpha)) {
            ttMove.value = NO_MOVE;
            addEntry(hashEntry, board->hash, &ttMove, depth, score, bound);
            return score;
        }
    }
    
    // Reverse futility pruning
    // Prune if the evaluation minus a margin
    // is greater than beta
    // The margin is multiplied by the depth
    // so that in deeper positions we consider
    // more moves
    if (!PVNode &&
        depth < 6 &&
        eval - (futilityMargin * (depth)) >= beta)
        return eval - futilityMargin;

    // Null move pruning
    // If making a null move (move that does nothing)
    // raises the score above beta, then cutoff
    if (!PVNode &&
        !inCheck &&
        eval >= beta &&
        hasNonPawnMaterial(board)) {
        
        R = depth > 6 ? 4 : 3;

        Undo undo = makeNullMove(board);

        score = -alphaBeta(board, -alpha-1, -alpha, depth-R-1, height+1, thread, &lastPv);

        undoNullMove(board, undo);

        if (score >= beta) {
            return score;
        }
    }

    initMovePicker(&mp, board, &ttMove, height);

    // Iterate over every move in the move list
    while ((move = selectNextMove(&mp, thread->hTable, board, 0)).value != NO_MOVE) {

        isQuiet = !moveIsTactical(&move, board);

        // Make the move
        // If the move is not legal, undo the move
        if (!makeMove(board, &move)) {
            undoMove(board, &move, &mp.undo);
            continue;
        }

        movecnt++;

        if (RootNode && thread->index == 0 && timeSearched(thread) > REPORT_TIME) {
            reportMoveInfo(move, *board, movecnt);
        }

        R = 0;

        // Late Move Reductions (LMR)
        // On later moves, reduce the depth of
        // the search
        // Because of move ordering later moves
        // tend to be worse, but this is not
        // always the case
        if (depth > 2 &&
            movecnt > 2) {
            R = reductionTable[MIN(depth, 63)][MIN(movecnt, 63)];

            R += isQuiet;

            int RDepth = CLAMP(depth-R-1, 1, depth-1);

            // Do a zero window search with the new depth
            score = -alphaBeta(board, -alpha-1, -alpha, RDepth, height+1, thread, &lastPv);

            didLMR = 1;
        } else {
            didLMR = 0;
        }

        // If the score after LMR is greater than alpha OR
        // if we are not doing LMR and this is not the first move
        // or we are in a zero window search, research the position
        // at full depth with zero window
        if ((didLMR && score > alpha) || (!didLMR && (movecnt > 1 || !PVNode))) {
            score = -alphaBeta(board, -alpha-1, -alpha, depth-1, height+1, thread, &lastPv);
        }

        // If we are in a full window search and the score of the
        // zero window search is greater than alpha OR
        // this is the first move search the position at full depth
        // with full windows
        if (PVNode && ((score > alpha && score < beta) || movecnt == 1)) {
            score = -alphaBeta(board, -beta, -alpha, depth-1, height+1, thread, &lastPv);
        }

        // Undo the move after searching the position
        undoMove(board, &move, &mp.undo);

        // If the score is our best score, update
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;

            // Weiss logic
            // Update our PV if we raised alpha in a PVNode,
            // or if this is the first move in the RootNode
            if ((score > alpha && PVNode) || (RootNode && movecnt == 1)) {
                pv->length = lastPv.length + 1;
                pv->pv[0] = move;
                memcpy(pv->pv+1, lastPv.pv, sizeof(Move) * lastPv.length);
            }

            // If the score is greater than alpha, update it
            if (score > alpha) {
                alpha = score;

                // Beta cutoff
                if (alpha >= beta) {
                    updateHistoryScores(thread->hTable, board, &bestMove, depth, height);
                    break;
                } 
            }
        }
        
    }

    // Check for checkmate/stalemate
    if (!movecnt) {
        bestScore = inCheck ? -999999+height : 0;
    }

    // Store TT entry
    addEntry(hashEntry, board->hash, &bestMove, depth, bestScore, EXACT);

    return bestScore;

}

void *startSearch(void *args) {
    Thread *thread = (Thread*)args;
    // Set our starting windows
    Score score, 
        alpha = -INF, beta = INF;
    int delta;
    int masterThread = thread->index == 0;
    for (thread->depth = 0; thread->depth < thread->maxDepth; thread->depth++) {
        if (STOP_SEARCH || setjmp(thread->jumpEnv)) break;
        PrincipalVariation pv;

        // Aspiration windows
        // We want to have smaller windows
        // so that we get more cutoffs in the search
        // However, if the windows are too small we can
        // miss moves
        // We will gradually widen the windows until we get a size that we want

        delta = 14;

        // We expect that our score will be
        // close to the score we got in the
        // last iteration

        if (thread->depth >= 5) {

            alpha = MAX(-999999, thread->score - delta);
            beta = MIN(999999, thread->score + delta);

        }

        while (1) {

            score = alphaBeta(&thread->board, alpha, beta, thread->depth, 0, thread, &pv);

            if (score <= alpha) {
                beta = (alpha+beta)/2;
                alpha = MAX(score - delta, -999999);
            } else if (score >= beta) {
                beta = MIN(score + delta, 999999);
            } else
                break;

            delta += delta/4;

        }
        
        thread->score = score;
        thread->pv = pv;

        if (!masterThread) {
            continue;
        }

        // Update time management after every depth
        // as master only
        updateTimeManagement(thread);

        reportSearchInfo(thread->threads);

    }

    // Report search info for the last time after breaking from a search
    if (masterThread) reportSearchInfo(thread->threads);

    return NULL;
}

Move getBestMove(Pos board, Thread *threads) {

    threads->tt->curAge++;

    pthread_t searchThreads[threads->threadCount];

    for (int i = 1; i < threads->threadCount; i++) {
        // Create all other threads (already created main thread)
        pthread_create(&searchThreads[i], NULL, &startSearch, (void*)&threads[i]);
    }
    startSearch((void*)&threads[0]);

    STOP_SEARCH = 1;
    for (int i = 1; i < threads->threadCount; i++) {
        pthread_join(searchThreads[i], NULL);
    }

    STOP_SEARCH = 0;

    return threads[0].pv.pv[0];
}
