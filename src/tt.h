#pragma once

#include "types.h"

struct ttEntry {
    Key key;
    Move move;
    int depth;
    int eval;
    int type;
    int age;
};

struct tTable {
    ttEntry *table;
    int entryCount;
    int curAge;
};

enum entryTypes {
    EXACT,
    UPPER, LOWER
};

void clearTT(tTable *tt);
void initTT(tTable *tt, int size);
void addEntry(ttEntry *entry, Key key, Move move, int depth, int eval, int type);
ttEntry *probeTT(tTable *tt, Key key, int *isReplaced);
