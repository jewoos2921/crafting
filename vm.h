//
// Created by jewoo on 2022-11-25.
//

#ifndef CRAFTING_VM_H
#define CRAFTING_VM_H

#include "chunk.h"

typedef struct {
    Chunk *chunk;
} VM;

void initVM();

void freeVM();

#endif //CRAFTING_VM_H
