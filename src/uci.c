#include "attacks.h"
#include "bitboards.h"
#include "evaluate.h"
#include "position.h"
#include "movegen.h"
#include "movepicker.h"
#include "search.h"
#include "threads.h"
#include "time.h"
#include "types.h"
#include "uci.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAX_DEPTH 2048

const char* startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

extern volatile int STOP_SEARCH;

extern Key zobristPieces[PIECE_CNT][SQ_CNT];
extern Key zobristEnPas[8];
extern Key zobristCastle[15];
extern Key zobristTurn;

void parseFen(const char* fen, Pos* board) {

    // Learn about what a FEN is here
    // https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation

    // Starting FEN
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

    // Counts the square currently being edited
    int square = 63;

    // Adds pieces to the board
    while (*fen && square >= 0) {
        switch (*fen) {

            // Add the piece to the board
            case 'k':
                setBit(&board->pieces[KING], flipSq(square));
                setBit(&board->sides[BLACK], flipSq(square));
                board->pieceList[flipSq(square)] = bK;
                board->hash ^= zobristPieces[bK][flipSq(square)];
                square--;
                break;

            case 'q':
                setBit(&board->pieces[QUEEN], flipSq(square));
                setBit(&board->sides[BLACK], flipSq(square));
                board->pieceList[flipSq(square)] = bQ;
                board->hash ^= zobristPieces[bQ][flipSq(square)];
                square--;
                break;

            case 'r':
                setBit(&board->pieces[ROOK], flipSq(square));
                setBit(&board->sides[BLACK], flipSq(square));
                board->pieceList[flipSq(square)] = bR;
                board->hash ^= zobristPieces[bR][flipSq(square)];
                square--;
                break;

            case 'b':
                setBit(&board->pieces[BISHOP], flipSq(square));
                setBit(&board->sides[BLACK], flipSq(square));
                board->pieceList[flipSq(square)] = bB;
                board->hash ^= zobristPieces[bB][flipSq(square)];
                square--;
                break;

            case 'n':
                setBit(&board->pieces[KNIGHT], flipSq(square));
                setBit(&board->sides[BLACK], flipSq(square));
                board->pieceList[flipSq(square)] = bN;
                board->hash ^= zobristPieces[bN][flipSq(square)];
                square--;
                break;

            case 'p':
                setBit(&board->pieces[PAWN], flipSq(square));
                setBit(&board->sides[BLACK], flipSq(square));
                board->pieceList[flipSq(square)] = bP;
                board->hash ^= zobristPieces[bP][flipSq(square)];
                square--;
                break;

            case 'K':
                setBit(&board->pieces[KING], flipSq(square));
                setBit(&board->sides[WHITE], flipSq(square));
                board->pieceList[flipSq(square)] = wK;
                board->hash ^= zobristPieces[wK][flipSq(square)];
                square--;
                break;

            case 'Q':
                setBit(&board->pieces[QUEEN], flipSq(square));
                setBit(&board->sides[WHITE], flipSq(square));
                board->pieceList[flipSq(square)] = wQ;
                board->hash ^= zobristPieces[wQ][flipSq(square)];
                square--;
                break;

            case 'R':
                setBit(&board->pieces[ROOK], flipSq(square));
                setBit(&board->sides[WHITE], flipSq(square));
                board->pieceList[flipSq(square)] = wR;
                board->hash ^= zobristPieces[wR][flipSq(square)];
                square--;
                break;

            case 'B':
                setBit(&board->pieces[BISHOP], flipSq(square));
                setBit(&board->sides[WHITE], flipSq(square));
                board->pieceList[flipSq(square)] = wB;
                board->hash ^= zobristPieces[wB][flipSq(square)];
                square--;
                break;

            case 'N':
                setBit(&board->pieces[KNIGHT], flipSq(square));
                setBit(&board->sides[WHITE], flipSq(square));
                board->pieceList[flipSq(square)] = wN;
                board->hash ^= zobristPieces[wN][flipSq(square)];
                square--;
                break;

            case 'P':
                setBit(&board->pieces[PAWN], flipSq(square));
                setBit(&board->sides[WHITE], flipSq(square));
                board->pieceList[flipSq(square)] = wP;
                board->hash ^= zobristPieces[wP][flipSq(square)];
                square--;
                break;

            // Skip a certain amount of squares based on the number

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                square -= (*fen - '0');
                break;

            // Ignore these characters

            case '/':
            case ' ':
                break;

            // Give an error message if using an invalid character
            default:
                printf("Error parsing FEN, pieces\n");
                break;
        }
        fen++;
    }

    // Turn

    fen++;

    board->turn = (*fen == 'w') ? WHITE : BLACK;
    if (board->turn) board->hash ^= zobristTurn;

    fen += 2;

    // Castle perms

    for (int i = 0; i < 4; i++) {
        if (*fen == ' ') break;
        switch(*fen) {
            case 'K': board->castlePerms |= CAN_WHITE_KING; board->hash ^= zobristCastle[CAN_WHITE_KING]; break;
            case 'Q': board->castlePerms |= CAN_WHITE_QUEEN; board->hash ^= zobristCastle[CAN_WHITE_QUEEN]; break;
            case 'k': board->castlePerms |= CAN_BLACK_KING; board->hash ^= zobristCastle[CAN_BLACK_KING]; break;
            case 'q': board->castlePerms |= CAN_BLACK_QUEEN; board->hash ^= zobristCastle[CAN_BLACK_QUEEN]; break;
            case '-': board->castlePerms = 0; break;
            default:
                printf("Error parsing FEN, castling\n");
                break;
        }
        fen++;
    }
    fen++;

    // En pas

    if (*fen != '-') {
        board->enPas = sq(fen[0] - 'a', fen[1] - '1');
        board->hash ^= zobristEnPas[file(board->enPas)];
        fen++;
    }
    fen += 2;

    char *ptr = strtok(strdup(fen), " ");

    // Half moves
    board->fiftyMoveRule = atoi(ptr);

    ptr = strtok(NULL, " ");

    // Full moves
    board->plyLength = atoi(ptr);
}


void intToSquare(int square, char *string) {
    *string++ = file(square) + 'a';
    *string++ = rank(square) + '1';
}

void moveToStr(Move move, Pos board, char *string) {
    int from = moveFrom(move);
    int to = moveTo(move);

    intToSquare(from, &string[0]);
    intToSquare(to, &string[2]);

    //promotion thing
    switch (promotePiece(move)) {
        case QUEEN:
            string[4] = 'q';
            break;
        case ROOK:
            string[4] = 'r';
            break;
        case KNIGHT:
            string[4] = 'n';
            break;
        case BISHOP:
            string[4] = 'b';
            break;
        default:
            string[4] = '\0';
            break;
    }
}


// Warning bad code
int strToFile(char *str) {
    int file;

    switch (*str) {
        case 'a':
            file = 0;
            break;
        case 'b':
            file = 1;
            break;
        case 'c':
            file = 2;
            break;
        case 'd':
            file = 3;
            break;
        case 'e':
            file = 4;
            break;
        case 'f':
            file = 5;
            break;
        case 'g':
            file = 6;
            break;
        case 'h':
            file = 7;
            break;
        default:
            file = -1;
            printf("something happened with a file not being valid");
            break;
    }
    return file;
}

Move strToMove(Pos board, char *str) {

    MoveList moves;
    moves.count = 0;

    char moveStr[6];
    
    genMoves(&moves, &board, ALLMOVES);
    
    for (int i = 0; i < moves.count; i++) {
        moveToStr(moves.moves[i], board, moveStr);
        if (strstr(str, moveStr) == str) {
            return moves.moves[i];
        }
    }

    printf("Invalid move %s\n", str);
    printBoard(board);

    Move a;
    a.value = NO_MOVE;

    return a;
}

void position(char* str, Pos* board) {
    resetBoard(board);
    str += 9;

    char* ptr;

    if (strstr(str, "startpos") == str) {
        parseFen(startingFen, board);
    } else if ((ptr = strstr(str, "fen")) == str) {
        if (!ptr){
            parseFen(startingFen, board);
        } else {
            ptr += 4;
            parseFen(ptr, board);
        }
    }

    ptr = strstr(str, "moves");

    if (ptr) {

        // Heres some really bad code i made a while ago in an old version
        // of this engine
        Move move;

        char *pointer = strtok(ptr, " ");

        for (pointer = strtok(NULL, " "); pointer != NULL; pointer = strtok(NULL, " ")) {

            move = strToMove(*board, pointer);

            makeMove(board, move);

            while (*(ptr) == ' ') ptr++;
        }

    }
}

void *go(void *args) {
    Pos board = ((goArgs*)args)->board;
    char *str = ((goArgs*)args)->str;
    Thread *threads = ((goArgs*)args)->threads;

    int wtime = -1;
    int btime = -1;
    int winc = -1;
    int binc = -1;
    int movestogo = 40;
    int movetime = -1;
    int maxDepth = MAX_DEPTH;

    Search info;

    info.infinite = 0;
    info.ponder = 0;
    info.maxDepth = maxDepth;
    
    if (strstr(str, "infinite") != NULL) {
        info.infinite = 1;
    } else if (strstr(str, "ponder") != NULL) {
        info.ponder = 1;
    }

    char *pointer = strtok(str, " ");

    for (pointer = strtok(NULL, " "); pointer != NULL; pointer = strtok(NULL, " ")) {
        if (strcmp(pointer, "depth") == 0) {
            maxDepth = atoi(strtok(NULL, " "));
        } else if (strcmp(pointer, "wtime") == 0) {
            wtime = atoi(strtok(NULL, " "));
        } else if (strcmp(pointer, "btime") == 0) {
            btime = atoi(strtok(NULL, " "));
        } else if (strcmp(pointer, "winc") == 0) {
            winc = atoi(strtok(NULL, " "));
        } else if (strcmp(pointer, "binc") == 0) {
            binc = atoi(strtok(NULL, " "));
        } else  if (strcmp(pointer, "movestogo") == 0) {
            movestogo = atoi(strtok(NULL, " "));
        } else  if (strcmp(pointer, "movetime") == 0) {
            movetime = atoi(strtok(NULL, " "));
        } else if (strcmp(pointer, "nodes") == 0) {
            info.maxNodes = atoi(strtok(NULL, " "));
        } else if (strcmp(pointer, "ponder") == 0) {
            info.ponder = 1;
        } else if (strcmp(pointer, "infinite") == 0) {
            info.infinite = 1;
        }
    }

    info.stopped = 0;
    info.nodes = 0;
    info.maxDepth = maxDepth;
    info.startTime = getTime();
    info.time = (movetime != -1 ? movetime : (board.turn ? (btime/movestogo + binc) : (wtime/movestogo + winc))); // change this later

    initThreadSearch(threads, board, info);
    
    Move bestMove = getBestMove(board, threads);

    char moveStr[6];
    moveToStr(bestMove, board, moveStr);
    printf("bestmove %s\n", moveStr);

    return NULL;
}

void setOption(char *str, Thread **threads, tTable *tt, HistoryTable *hTable) {

    if (strstr(str, "setoption name Threads value") == str) {
        free(*threads);
        *threads = initThreads(atoi(str + 28), tt, hTable);
    } else if (strstr(str, "setoption name Hash value") == str) {
        initTT(tt, atoi(str + 25));
    }
}

// Bench used for benchmark results from
// OpenBench

static const char *BenchFENs[] = {
    #include "bench.csv"
    ""
};

void bench(int argc, char **argv) {

    Pos board;
    resetBoard(&board);
    parseFen(startingFen, &board);

    tTable tt;
    Thread *threads;

    HistoryTable hTable;

    Key nodes = 0ull;

    int depth = argc > 2 ? atoi(argv[2]) : 6;
    int threadCount = argc > 3 ? atoi(argv[3]) : 1;
    int ttSize = argc > 4 ? atoi(argv[4]) : 16;

    initTT(&tt, ttSize);
    threads = initThreads(threadCount, &tt, &hTable);

    initHistoryTable(&hTable);

    int startTime = getTime();

    Search info;
    info.startTime = startTime;
    info.time = 999999999;
    info.maxDepth = depth;
    info.infinite = 0;

    for (int i = 0; strcmp(BenchFENs[i], ""); i++) {
        resetBoard(&board);
        parseFen(BenchFENs[i], &board);

        initThreadSearch(threads, board, info);
        printf("Position %d: %s\n", i+1, BenchFENs[i]);

        getBestMove(board, threads);

        for (int i = 0; i < threads->threadCount; i++) {
            nodes += threads[i].nodes;
        }

        clearTT(&tt);
    }

    int endTime = getTime();

    printf("Time  : %dms\n", endTime - startTime);
    printf("Nodes : %llu\n", nodes);
    printf("NPS   : %d\n", (int)(nodes*1000 / (endTime - startTime)));

    free(threads);
}

void uciLoop() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    Pos board;
    resetBoard(&board);
    parseFen(startingFen, &board);

    char str[2048];

    goArgs goArgs;

    tTable tt;
    initTT(&tt, 1);

    HistoryTable hTable;

    initHistoryTable(&hTable);

    pthread_t goThread;
    Thread *threads = initThreads(1, &tt, &hTable);

    int QUIT_SIGNAL = 1;

    do {
        fflush(stdout);
        if (!fgets(str, 2048, stdin))
            continue;

        if (strcmp(str, "quit\n") == 0) {
            QUIT_SIGNAL = 0;
        } else if (strcmp(str, "uci\n") == 0) {
            printf("id name Omelette\n");
            printf("id author Benjamin Paul\n");
            printf("option name Threads type spin default 1 min 1 max 512\n");
            printf("option name Hash type spin default 1 min 1 max 65536\n");
            printf("uciok\n");
        } else if (strcmp(str, "isready\n") == 0) {
            printf("readyok\n");
        } else if (strstr(str, "position") == str) {
            position(str, &board);
        } else if (strcmp(str, "print\n") == 0) {
            printBoard(board);
        } else if (strstr(str, "perft") == str) {

            char* ptr = str;
            ptr+=6;

            Bitboard a = perft(board, atoi(ptr), 1);

            printf("%llu\n", a);

        } else if (strstr(str, "divide") == str) {

            char* ptr = str;
            ptr+=7;

            int startTime = getTime();

            char mov[6];

            MoveList moves;
            moves.count = 0;
            genMoves(&moves, &board, ALLMOVES);
            
            for (int i = 0; i < moves.count; i++) {
                if (!makeMove(&board, moves.moves[i])) {
                    continue;
                }

                Bitboard a = perft(board, atoi(ptr), 1);

                undoMove(&board, moves.moves[i]);

                moveToStr(moves.moves[i], board, mov);
                printf("%s: %llu\n", mov, a);

            }

            int endTime = getTime();

            printf("time taken: %dms\n", endTime - startTime);

        } else if (strcmp(str, "test\n") == 0) {

            // test

        } else if (strstr(str, "move") == str) {
            char *ptr = str;
            ptr+= 5;

            Move move;
            char moveStr[6];

            while (ptr != NULL && *(ptr) != '\0') {
                for (int i = 0; i < 4; i++) {
                    moveStr[i] = *(ptr)++;
                }
                moveStr[4] = *ptr == '\0' || *ptr == ' ' ? '\0' : *(ptr)++;
                moveStr[5] = '\0';

                if (!moveStr[0]) break;


                move = strToMove(board, moveStr);

                makeMove(&board, move);

                while (*(ptr) == ' ') ptr++;
            }

        } else if (strstr(str, "go") == str) {
            strncpy(goArgs.str, str, 2048);
            goArgs.board = board;
            goArgs.threads = threads;
            pthread_create(&goThread, NULL, &go, &goArgs);
        } else if (strstr(str, "stop") == str) {
            STOP_SEARCH = 1;
        } else if (strstr(str, "eval") == str) {
            printf("%d\n", evaluate(board));
        } else if (strstr(str, "setoption") == str) {
            setOption(str, &threads, &tt, &hTable);
        }

    } while (QUIT_SIGNAL);
}


void reportSearchInfo(Thread *threads) {
    long nodes = 0;
    char moveStr[6];

    for (int i = 0; i < threads->threadCount; i++) {
        nodes += threads[i].nodes;
    }

    int curTime = getTime();
    long nps = nodes*1000/(curTime - threads[0].startTime+1);
    
    printf("info depth %d seldepth %d nodes %lu nps %lu pv", threads[0].depth, threads[0].seldepth, nodes, nps);

    for (int i = 0; i < threads[0].pv.length; i++) {
        moveToStr(threads[0].pv.pv[i], threads[0].board, moveStr);

        printf(" %s", moveStr);
    } 

    printf(" score ");

    if (abs(threads[0].score) < 997951) {
        printf("cp %d", threads[0].score);
    } else {
        int absVal = (999999-abs(threads[0].score))/2 + 1;
        printf("mate %d", threads[0].score > 0 ? absVal : -absVal);
    }

    printf("\n");
    
}

void reportMoveInfo(Move move, Pos board, int index) {
    char moveStr[6];
    moveToStr(move, board, moveStr);
    printf("info currmove %s currmovenumber %d\n", moveStr, index);
}
