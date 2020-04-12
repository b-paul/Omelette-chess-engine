#pragma once

typedef unsigned long long Bitboard;
typedef unsigned long long Key;

#define max(a, b) (((a) > (b)) ? a : b)
#define min(a, b) (((a) < (b)) ? a : b)

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQ,

    SQ_CNT = 64
};

enum File {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,

    FILE_CNT = 8
};

enum Rank {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,

    RANK_CNT = 8
};

enum Colour {
    WHITE, BLACK, CL_CNT = 2
};

enum Piece {
    NONE,

    // Numbers are to optimize the piece to pieceType function
    wP = 1, wN, wB, wR, wQ, wK,
    bP = 9, bN, bB, bR, bQ, bK,

    PIECE_CNT = 16
};

enum PieceType {
    PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING,

    PIECE_TYPE_CNT = 7
};

enum CastlePerms {
    WHITE_KING_OCC, WHITE_KING_ATTACK, WHITE_QUEEN_OCC, WHITE_QUEEN_ATTACK, 
    BLACK_KING_OCC, BLACK_KING_ATTACK, BLACK_QUEEN_OCC, BLACK_QUEEN_ATTACK,

    CASTLE_CNT
};

enum CastlePermBits {
    CAN_WHITE_KING = 1,
    CAN_WHITE_QUEEN = 2,
    CAN_WHITE = 3,
    CAN_BLACK_KING = 4,
    CAN_BLACK_QUEEN = 8,
    CAN_BLACK = 12
};

enum MoveType {
    QUIET, NOISY, ALLMOVES
};

enum MoveFlags {
    NORMAL,
    PROMOTE = 1 << 15,
    ENPAS = 2 << 15,
    CASTLE = 3 << 15
};

enum Moves {
        NO_MOVE,
        NULL_MOVE = 65,
};

typedef struct goArgs goArgs;
typedef struct Magic Magic;
typedef struct Move Move;
typedef struct MoveList MoveList;
typedef struct MovePicker MovePicker;
typedef struct Pos Pos;
typedef struct PrincipalVariation PrincipalVariation;
typedef struct Search Search;
typedef struct Thread Thread;
typedef struct ttEntry ttEntry;
typedef struct tTable tTable;
