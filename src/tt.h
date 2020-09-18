#pragma once

#include "movegen.h"
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
    UPPER = 1, LOWER,
    EXACT
};

void clearTT(tTable *tt);
void initTT(tTable *tt, const int size);
void addEntry(ttEntry *entry, Key key, Move *move, const int depth, const int eval, const int type);
ttEntry *probeTT(tTable *tt, Key key, int *isReplaced);
int hashFull(tTable *tt);
