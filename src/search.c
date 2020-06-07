#include "evaluate.h"
#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "threads.h"
#include "time.h"
#include "tt.h"
#include "types.h"
#include "uci.h"

#include <math.h>
#include <pthread.h>
#include <string.h>

volatile int STOP_SEARCH = 0;

int futilityMargin = 300;

Bitboard perft(Pos *board, int depth, int isRoot) {
    if (depth == 0) return 1;

    MoveList moves;
    moves.count = 0;
    genMoves(&moves, board, ALLMOVES);

    int nodes = 0;

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
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++) {
            reductionTable[i][j] = 1 + (int)((log(i)*log(j))/10);
        }
}

int qsearch(Pos *board, int alpha, int beta, int height, Thread *thread, PrincipalVariation *pv) {
    thread->nodes++;
    if (height > thread->seldepth) thread->seldepth = height;

    if (STOP_SEARCH || timeLeft(thread) <= 0) longjmp(thread->jumpEnv, 1);

    int standPat = evaluate(board);

    if (standPat >= beta)
        return standPat;

    if (alpha < standPat)
        alpha = standPat;

    int score;
    int bestScore = standPat;
    Move move;

    PrincipalVariation lastPv;
    lastPv.length = 0;
    pv->length = 0;

    Move ttMove;
    ttMove.value = NO_MOVE;

    MovePicker mp;

    initMovePicker(&mp, board, &ttMove, height);

    while ((move = selectNextMove(&mp, thread->hTable, board, 1)).value != NO_MOVE) {
        if (!makeMove(board, &move)) {
            undoMove(board, &move, &mp.undo);
            continue;
        }

        score = -qsearch(board, -beta, -alpha, height+1, thread, &lastPv);
        undoMove(board, &move, &mp.undo);

        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                alpha = score;
                
                pv->length = lastPv.length + 1;
                pv->pv[0] = move;
                memcpy(pv->pv+1, lastPv.pv, sizeof(Move) * lastPv.length);

                if (alpha >= beta) {
                    break;
                } 
            }
        }
    }

    return bestScore;
}

int alphaBeta(Pos *board, int alpha, int beta, int depth, int height, Thread *thread, PrincipalVariation *pv) {
    if (depth <= 0) {
        return qsearch(board, alpha, beta, height, thread, pv);
    }

    thread->nodes++;
    if (height > thread->seldepth) thread->seldepth = height;

    if (STOP_SEARCH || timeLeft(thread) <= 0) longjmp(thread->jumpEnv, 1);

    // check for draws
    if (isDrawn(board, height)) return 0;

    PrincipalVariation lastPv;
    lastPv.length = 0;
    pv->length = 0;

    int score,bestScore=-999999;
    int movecnt = 0;
    Move move, bestMove;

    // PVS sets alpha to beta-1 on
    // non-PV nodes
    int PVNode = alpha != beta-1;

    int RootNode = height == 0;

    int eval = evaluate(board);

    int isQuiet;
    int R, didLMR;

    int inCheck = squareAttackers(board, getlsb(board->pieces[KING] & board->sides[board->turn]), board->turn) ? 1 : 0;

    MovePicker mp;

    ttEntry *hashEntry;
    int isReplaced;
    int ttEval;
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

    // Null move pruning
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

    while ((move = selectNextMove(&mp, thread->hTable, board, 0)).value != NO_MOVE) {

        isQuiet = !moveIsTactical(&move, board);

        if (!makeMove(board, &move)) {
            undoMove(board, &move, &mp.undo);
            continue;
        }

        movecnt++;

        if (RootNode && thread->index == 0) {
            reportMoveInfo(move, *board, movecnt);
        }

        R = 0;

        // LMR
        if (depth > 2 &&
            movecnt > 2) {
            R = reductionTable[min(depth, 63)][min(movecnt, 63)];

            R += isQuiet;

            int RDepth = clamp(depth-R-1, 1, depth-1);

            score = -alphaBeta(board, -alpha-1, -alpha, RDepth, height+1, thread, &lastPv);

            didLMR = 1;
        } else {
            didLMR = 0;
        }

        if ((didLMR && score > alpha) || (!didLMR && (movecnt > 1 || !PVNode))) {
            score = -alphaBeta(board, -alpha-1, -alpha, depth-1, height+1, thread, &lastPv);
        }

        if (PVNode && ((score > alpha && score < beta) || movecnt == 1)) {
            score = -alphaBeta(board, -beta, -alpha, depth-1, height+1, thread, &lastPv);
        }

        undoMove(board, &move, &mp.undo);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;

            if (score > alpha) {
                alpha = score;

                if (PVNode) {
                    pv->length = lastPv.length + 1;
                    pv->pv[0] = move;
                    memcpy(pv->pv+1, lastPv.pv, sizeof(Move) * lastPv.length);
                }

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

    addEntry(hashEntry, board->hash, &bestMove, depth, bestScore, EXACT);

    return bestScore;

}

void *startSearch(void *args) {
    Thread *thread = (Thread*)args;
    int score, 
        alpha = -999999, beta = 999999, delta;
    int masterThread = thread->index == 0;
    for (thread->depth = 0; thread->depth < thread->maxDepth; thread->depth++) {
        if (STOP_SEARCH) break;
        if (setjmp(thread->jumpEnv)) break;
        PrincipalVariation pv;

        delta = 24;

        // We expect that our score will be
        // close to the score we got in the
        // last iteration

        if (thread->depth >= 5) {

            alpha = max(-999999, thread->score-delta);
            beta = min(999999, thread->score+delta);

        }

        while (1) {

            score = alphaBeta(&thread->board, alpha, beta, thread->depth, 0, thread, &pv);

            if (score <= alpha) {
                beta = (alpha+beta)/2;
                alpha = max(score - delta, -999999);
            } else if (score >= beta) {
                beta = min(score + delta, 999999);
            } else
                break;

            delta += delta/4;

        }
        
        thread->score = score;
        thread->pv = pv;

        if (!masterThread) {
            continue;
        }

        reportSearchInfo(thread->threads);

    }

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
