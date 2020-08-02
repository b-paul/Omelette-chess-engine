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
    if (key != entry->key || depth >= entry->depth || type == EXACT) {
        entry->key = key;
        entry->move = *move;
        entry->depth = depth;
        entry->eval = eval;
        entry->type = type;
    }
}

ttEntry *probeTT(tTable *tt, Key key, int *isReplaced) {

    // We pretty much will never have
    // tt entryCount > key even with
    // a supercomputer using all of
    // its memory
    int i = key % tt->entryCount;

    ttEntry *entry = &tt->table[i];

    *isReplaced = entry->key == key;

    return entry;
}

int hashFull(tTable *tt) {
    int count = 0;
    int entries = 1000;

    for (int i = 0; i < entries; i++)
        if (tt->table[i].move.value != NO_MOVE)
            count++;

    return count;
}
