#include "movegen.h"
#include "tt.h"
#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

void clearTT(tTable *table) {
    for (int i = 0; i < table->entryCount; i++) {
        table->table[i].key = 0;
        table->table[i].move.value = NO_MOVE;
        table->table[i].move.score = 0;
        table->table[i].depth = 0;
        table->table[i].eval = 0;
        table->table[i].type = 0;
    }
}

// Size is in MB
void initTT(tTable *table, int size) {

    table->curAge = 0;

    // Convert to bytes
    long sizeBytes = size * 1048576;

    table->entryCount = sizeBytes/sizeof(ttEntry);

    // Clear whats in the table if there
    // is anything
    if (table->table != NULL)
        free(table->table);

    table->table = (ttEntry*)aligned_alloc(1048576, sizeBytes);
    madvise(table->table, size, MADV_HUGEPAGE);

    memset(table->table, 0, sizeBytes);

    if (table->table == NULL)
        printf("Failed to allocate %d MB to the hash table\n", size);

    clearTT(table);

}

void addEntry(ttEntry *entry, Key key, Move *move, int depth, int eval, int type) {
    entry->key = key;
    entry->move = *move;
    entry->depth = depth;
    entry->eval = eval;
    entry->type = type;
}

ttEntry *probeTT(tTable *tt, Key key, int *isReplaced) {
    *isReplaced = 1;

    // We pretty much will never have
    // tt entryCount > key even with
    // a supercomputer using all of
    // its memory
    int i = key % tt->entryCount;

    if (tt->table[i].key == key) {
        return &tt->table[i];
    }

    *isReplaced = 0;

    // We did not find an entry with our
    // key, to we will pick an empty or
    // replace another entry

    ttEntry *entry = NULL;

    for (i = 0; i < tt->entryCount; i++) {
        if (tt->table[i].key == 0) {
            entry = &tt->table[i];
            break;
        }
        if (tt->table[i].age < tt->curAge) {
            entry = &tt->table[i];
            break;
        }
    }

    if (entry == NULL) {
        // bad approach that
        // should be fixed
        entry = &tt->table[0];
    }

    return entry;
}
