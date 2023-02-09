//
// Created by jewoo on 2023-01-14.
//

#ifndef CRAFTING_SYNCHRONIZATION_H
#define CRAFTING_SYNCHRONIZATION_H

#include "Types.h"

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// 뮤텍스 자료구조
typedef struct kMutexStruct {
    // 태스크 ID와 잠금을 수행한 횟수
    // volatile : 해당 변수에 접근할 때 매번 메모리를 참조하도록 함
    volatile QWORD qwTaskID;
    volatile DWORD dwLockCount;

    // 잠금 플래그
    volatile BOOL blockFlag;

    // 자료구조의 크기를 8바이트 단위로 맞추려고 추가한 필드
    BYTE vbPadding[3];
} MUTEX;

/// 스핀락 자료구조
typedef struct kSpinLockStruct {

    /// 로컬 APIC ID와 잠금을 수행한 횟수
    volatile DWORD dwLockCount;
    volatile BYTE bAPICD;

    /// 잠금 플래그
    volatile BOOL bLockFlag;

    /// 인터럽트 플래그
    volatile BOOL bInterruptFlag;

    /// 자료구조의 크기를 8바이트 단위로 맞추려고 추가한 필드
    BYTE vbPadding[1];
} SPINLOCK;

#pragma pack(pop)


// 함수
#if 0
BOOL kLockForSystemData(void);

void kUnlockForSystemData(BOOL bInterruptFlag);
#endif

void kInitializeSpinLock(SPINLOCK *pstSpinLock);

void kLockForSpinLock(SPINLOCK *pstSpinLock);

void kUnlockForSpinLock(SPINLOCK *pstSpinLock);

void kInitializeMutex(MUTEX *pstMutext);

void kLock(MUTEX *pstMutext);

void kUnlock(MUTEX *pstMutext);


#endif //CRAFTING_SYNCHRONIZATION_H
