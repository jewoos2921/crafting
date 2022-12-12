//
// Created by jewoo on 2022-12-12.
//

#ifndef CRAFTING_VALUE_H
#define CRAFTING_VALUE_H

#include "value.h"

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

void initValueArray(ValueArray *array);

void writeValueArray(ValueArray *array, Value value);

void freeValueArray(ValueArray *array);

void printValue(Value value);

#endif //CRAFTING_VALUE_H
