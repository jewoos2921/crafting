//
// Created by jewoo on 2022-11-24.
//

#ifndef CRAFTING_DEBUG_H
#define CRAFTING_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name);

int disassembleInstruction(Chunk *chunk, int offset);



#endif //CRAFTING_DEBUG_H
