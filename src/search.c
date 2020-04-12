#include "evaluate.h"
#include "movegen.h"
#include "movepicker.h"
#include "position.h"
#include "threads.h"
#include "time.h"
#include "tt.h"
#include "types.h"
#include "uci.h"

#include <pthread.h>
#include <string.h>

volatile int STOP_SEARCH = 0;

Bitboard perft(Pos board, int depth, int isRoot) {
  if (depth == 0) return 1;

  MoveList moves;
  moves.count = 0;
  genMoves(&moves, &board, ALLMOVES);

  int nodes = 0;

  for (int i = 0; i < moves.count; i++) {
    if (!makeMove(&board, moves.moves[i])) {
      continue;
    }
    nodes += perft(board, depth - 1, 0);
    undoMove(&board, moves.moves[i]);
  }
  return nodes;
}

int reductionTable[64][64];

void initSearch() {
    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 64; j++) {
            reductionTable[i][j] = 1;
        }
}

int qsearch(Pos board, int alpha, int beta, int height, Thread *thread, PrincipalVariation *pv) {
    if (height > thread->seldepth) thread->seldepth = height;
    thread->nodes++;

    if (STOP_SEARCH || timeLeft(*thread) <= 0) longjmp(thread->jumpEnv, 1);

    int standPat = evaluate(board);

    if (standPat >= beta)
        return standPat;

    if (alpha - 200 > standPat) return standPat;

    if (alpha < standPat)
        alpha = standPat;

    int score;
    int bestScore = standPat;
    Move move;

    PrincipalVariation lastPv;
    lastPv.length = 0;

    Move ttMove;
    // temp
    ttMove.value = NO_MOVE;

    MovePicker mp;

    initMovePicker(&mp, ttMove);

    while ((move = selectNextMove(&mp, &board, 1)).value != NO_MOVE) {
        if (!makeMove(&board, move))
            continue;
        score = -qsearch(board, -beta, -alpha, height+1, thread, &lastPv);
        undoMove(&board, move);

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

int alphaBeta(Pos board, int alpha, int beta, int depth, int height, Thread *thread, PrincipalVariation *pv) {
    if (depth <= 0) {
        return qsearch(board, alpha, beta, height, thread, pv);
    }

    if (height > thread->seldepth) thread->seldepth = height;
    thread->nodes++;

    if (STOP_SEARCH || timeLeft(*thread) <= 0) longjmp(thread->jumpEnv, 1);

    // check for draws
    if (isDrawn(board, height)) return 0;

    PrincipalVariation lastPv;
    lastPv.length = 0;

    int score,bestScore=-999999;
    int movecnt = 0;
    int searchPV = 1;
    Move move, bestMove;

    // PVS sets alpha to beta-1 on
    // non-PV nodes
    int PVNode = alpha != beta-1;

    MovePicker mp;

    ttEntry *hashEntry;
    int isReplaced;
    int ttEval;
    Move ttMove;

    // Probe TT
    hashEntry = probeTT(thread->tt, board.hash, &isReplaced);
    ttEval = isReplaced ? hashEntry->eval : 0;
    ttMove.value = isReplaced ? hashEntry->move.value : NO_MOVE;

    // Cutoff if we have searched this position
    // to a greater or equal depth
    if (!PVNode &&
        isReplaced &&
        hashEntry->depth >= depth) {
        return ttEval;
    }

    ttMove.value = NO_MOVE;

    initMovePicker(&mp, ttMove);

    while ((move = selectNextMove(&mp, &board, 0)).value != NO_MOVE) {
        if (!makeMove(&board, move))
            continue;


        movecnt++;

        // Reductions

        if (searchPV)
            score = -alphaBeta(board, -beta, -alpha, depth-1, height+1, thread, &lastPv);
        else {
            score = -alphaBeta(board, -alpha-1, -alpha, depth-1, height+1, thread, &lastPv);
            if (score > alpha)
                score = -alphaBeta(board, -beta, -alpha, depth-1, height+1, thread, &lastPv);
        }

        undoMove(&board, move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;

            if (score > alpha) {
                alpha = score;
                searchPV = 0;
            
                pv->length = lastPv.length + 1;
                pv->pv[0] = move;
                memcpy(pv->pv+1, lastPv.pv, sizeof(Move) * lastPv.length);

                if (alpha >= beta) {
                    break;
                } 
            }
        }
        
    }

    // Check for checkmate/stalemate
    if (!movecnt) {
        bestScore = squareAttackers(board, getlsb(board.pieces[KING] & board.sides[!board.turn]), board.turn) ? -999999+height : 0;
    }

    addEntry(hashEntry, board.hash, bestMove, depth, bestScore, EXACT);

    return bestScore;

}

void *startSearch(void *args) {
    Thread *thread = (Thread*)args;
    Pos board = thread->board;
    int score;
    int masterThread = thread->index == 0;
    for (thread->depth = 0; thread->depth < thread->maxDepth; thread->depth++) {
        if (STOP_SEARCH) break;
        if (setjmp(thread->jumpEnv)) break;
        PrincipalVariation pv;
        score = alphaBeta(board, -999999, 999999, thread->depth, 0, thread, &pv);

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

    return threads[0].pv.pv[0];
}
