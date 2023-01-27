//
// Created by jewoo on 2023-01-22.
//

// 해시 연산을 정의한 파일

#ifndef CRAFTING_HASH_H
#define CRAFTING_HASH_H

#include "BaseHeader.h"
#include "HashPage.h"

void HInitHash(int iMaxAddress, char *cpFileName, int iPageSize,
               BOOL bNewStart);

void HCloseHash(Key key);

BOOL hAdd(Key key, Value value);


#endif //CRAFTING_HASH_H
