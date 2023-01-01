//
// Created by jewoo on 2023-01-01.
//

#ifndef CRAFTING_UTILITY_H
#define CRAFTING_UTILITY_H

#include "Types.h"

void kMemSet(void *pvDestination, BYTE bData, int iSize);

int kMemCpy(void *pvDestination, const void *pvSource, int iSize);

int kMemCmp(const void *pvDestination, const void *pvSource, int iSize);

#endif //CRAFTING_UTILITY_H
