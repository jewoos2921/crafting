//
// Created by jewoo on 2022-12-12.
//

#ifndef CRAFTING_VM_H
#define CRAFTING_VM_H

#include "chunk.h"

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVM();

void freeVM();

InterpretResult interpret(Chunk *chunk);

#endif //CRAFTING_VM_H
