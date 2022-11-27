//
// Created by jewoo on 2022-11-24.
//

#ifndef CRAFTING_MEMORY_H
#define CRAFTING_MEMORY_H

#include "common.h"


#define ALLOCATE(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))


#define GROW_CAPACITY(capacity) \
        ((capacity) < 8 ? 8: (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
            (type*) reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))


#define FREE_ARRAY(type, pointer, oldCount) reallocate(pointer, sizeof(type) *(oldCount) ,0)


void *reallocate(void *pointer, size_t oldSize, size_t newSize);


#endif //CRAFTING_MEMORY_H
