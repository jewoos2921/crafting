//
// Created by jewoo on 2023-01-01.
//

#ifndef CRAFTING_UTILITY_H
#define CRAFTING_UTILITY_H

#include <stdarg.h>
#include "Types.h"

void kMemSet(void *pvDestination, BYTE bData, int iSize);

int kMemCpy(void *pvDestination, const void *pvSource, int iSize);

int kMemCmp(const void *pvDestination, const void *pvSource, int iSize);

BOOL kSetInterruptFlag(BOOL bEnableInterrupt);

int kStrLen(const char *pcBuffer);

void kCheckTotalRAMSize(void);

QWORD kGetTotalRAMSize(void);

void kReverseString(char *pcBuffer);

long kAToI(const char *pcBuffer, int iRadix);

QWORD kHexSringToQward(const char *pcBuffer);

long kDecimalStringToLong(const char *pcBuffer);

int kIToA(long lValue, char *pcBuffer, int iRadix);

int kHexToString(QWORD qwValue, char *pcBuffer);

int kDecimalToString(long lValue, char *pcBuffer);

int kSPrintf(char *pcBuffer, const char *pcFormatString, ...);

int kVSPrintf(char *pcBuffer, const char *pcFormatString, va_list ap);

QWORD kGetTickCount(void);

extern volatile QWORD g_qwTickCount;


#endif //CRAFTING_UTILITY_H
