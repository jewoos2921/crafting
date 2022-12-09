//
// Created by jewoo on 2022-12-09.
//

#ifndef CRAFTING_CHUNK_H
#define CRAFTING_CHUNK_H

#include "common.h"

typedef enum {
    OP_RETURN
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t *code;
} Chunk;

void initChunk(Chunk *chunk);

void freeChunk(Chunk * chunk);

void writeChunk(Chunk *chunk, uint8_t byte);

#endif //CRAFTING_CHUNK_H
