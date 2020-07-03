#pragma once

#include "evaluate.h"
#include "types.h"

#include <stdbool.h>

typedef enum {
    EXACT,
    UPPER, LOWER
} Bound;

struct ttEntry {
    Key key;
    Move move;
    int depth;
    Score eval;
    Bound type;
    int age;
};

struct tTable {
    ttEntry *table;
    int entryCount;
    int curAge;
};

void clearTT(tTable *tt);
void initTT(tTable *tt, int size);
void addEntry(ttEntry *entry, Key key, Move *move, int depth, Score eval, Bound type);
ttEntry *probeTT(tTable *tt, Key key, bool *isReplaced);
