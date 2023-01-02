//
// Created by jewoo on 2023-01-02.
//

#ifndef CRAFTING_INTERRUPTHANDLER_H
#define CRAFTING_INTERRUPTHANDLER_H

#include "Types.h"

void kCommonExceptionHanlder(int iVectorNumber, QWORD qwErrorCode);

void kCommonInterruptHandler(int iVectorNumber);

void kKeyboardHandler(int iVectorNumber);

#endif //CRAFTING_INTERRUPTHANDLER_H
