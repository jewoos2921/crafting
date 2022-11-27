//
// Created by jewoo on 2022-11-25.
//

#ifndef CRAFTING_COMPILER_H
#define CRAFTING_COMPILER_H

#include "object.h"
#include "vm.h"

bool compile(const char *source, Chunk *chunk);

#endif //CRAFTING_COMPILER_H
