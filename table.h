//
// Created by jewoo on 2022-11-27.
//

#ifndef CRAFTING_TABLE_H
#define CRAFTING_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjString *key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

void initTable(Table *table);

void freeTable(Table *table);

bool tableGet(Table *table, ObjString *key, Value *value);

bool tableSet(Table *table, ObjString *key, Value value);

void tableAddAll(Table *from, Table *to);

#endif //CRAFTING_TABLE_H
