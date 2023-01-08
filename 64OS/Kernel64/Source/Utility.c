//
// Created by jewoo on 2023-01-01.
//
#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>


// 메모리를 특정 캆으로 채움
void kMemSet(void *pvDestination, BYTE bData, int iSize) {
    int i;
    for (i = 0; i < iSize; ++i) {
        ((char *) pvDestination)[i] = bData;
    }
}

// 메모리 복사
int kMemCpy(void *pvDestination, const void *pvSource, int iSize) {
    int i;
    for (i = 0; i < iSize; ++i) {
        ((char *) pvDestination)[i] = ((char *) pvSource)[i];
    }
    return iSize;
}

// 메모리 비교
int kMemCmp(const void *pvDestination, const void *pvSource, int iSize) {
    int i;
    char cTemp;
    for (i = 0; i < iSize; ++i) {
        cTemp = ((char *) pvDestination)[i] - ((char *) pvSource)[i];
        if (cTemp != 0) {
            return (int) cTemp;
        }
    }
    return 0;
}

// RFLAGS 레지스터의 인터럽트 플래그를 변경하고 이전 이너럽트
BOOL kSetInterruptFlag(BOOL bEnableInterrupt) {

    QWORD qwRFLAGS;

    // 이전의 RFLAGS 레지스터 값을 읽은 뒤에 인터럽트 가능/불가 처리
    qwRFLAGS = kReadRFLAGS();
    if (bEnableInterrupt == TRUE) {
        kEnableInterrupt();
    } else {
        kDisableInterrupt();
    }

    // 이전 RFLAGS 레지스터의 IF 비트(비트 9)를 확인하여 이전의 인터럽트 상태를 반환
    if (qwRFLAGS & 0x0200) {
        return TRUE;
    }

    return FALSE;
}
