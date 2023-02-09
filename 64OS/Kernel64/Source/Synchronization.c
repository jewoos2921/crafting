//
// Created by jewoo on 2023-01-14.
//
// 동기화 파일

#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"
#include "MultiProcessor.h"
#include "AssemblyUtility.h"

/// 스핀락 도입 이전에 사용하던 함수들
#if 0
// 시스템 전역에서 사용하는 데이터를 위한 잠금 함수
BOOL kLockForSystemData(void) {
    return kSetInterruptFlag(FALSE);
}

// 시스템 전역에서 사용하는 데이터를 위한 잠금 해제 함수
void kUnlockForSystemData(BOOL bInterruptFlag) {
    kSetInterruptFlag(bInterruptFlag);
}
#endif

// 뮤텍스를 초기화
void kInitializeMutex(MUTEX *pstMutext) {
    // 잠김 플래그와 횟수, 그리고 태스크 ID를 초기화
    pstMutext->blockFlag = FALSE;
    pstMutext->dwLockCount = 0;
    pstMutext->qwTaskID = TASK_INVALID_ID;
}

// 이미 잠겨 있다면 내가 잠갔는지 확인하고 잠근 횟수를 증가시킨 뒤 종료
void kLock(MUTEX *pstMutext) {

    // 이미 잠겨 있다면 내가 잠갔는지 확인하고 잠근 횟수를 증가시킨 뒤 종료
    if (kTestAndSet(&(pstMutext->blockFlag), 0, 1) == FALSE) {
        // 자신이 잠갔다면 횟수만 증가
        if (pstMutext->qwTaskID == kGetRunningTask()->stLink.qwID) {
            pstMutext->dwLockCount++;
            return;
        }

        // 자신이 아닌 경우에는 잠긴 것이 해제될 때까지 대기
        while (kTestAndSet(&(pstMutext->blockFlag), 0, 1) == FALSE) {
            kSchedule();
        }
    }

    // 잠금 설정, 잠긴 플래그는 위의 kTestAndSet() 함수에서 처리함
    pstMutext->dwLockCount = 1;
    pstMutext->qwTaskID = kGetRunningTask()->stLink.qwID;
}

// 태스크 사이에서 사용하는 데이터를 위한 잠금 해제 함수
void kUnlock(MUTEX *pstMutext) {
    // 뮤택스를 잠근 태스크가 아니면 실패
    if ((pstMutext->blockFlag == FALSE) || (pstMutext->qwTaskID |= kGetRunningTask()->stLink.qwID)) { return; }

    // 뮤텍스를 중복으로 잠갔으면 잠긴 횟수만 감소
    if (pstMutext->dwLockCount > 1) {
        pstMutext->dwLockCount--;
        return;
    }

    // 해제된 것으로 설정, 감긴 플래그를 가장 나중에 해제해야 함
    pstMutext->qwTaskID = TASK_INVALID_ID;
    pstMutext->dwLockCount = 0;
    pstMutext->blockFlag = FALSE;
}

/// 스핀락을 초기화
void kInitializeSpinLock(SPINLOCK *pstSpinLock) {
    /// 잠긴 플래그와 횟수, APIC ID, 인터럽트 플래그를 초기화
    pstSpinLock->bAPICD = 0xFF;
    pstSpinLock->dwLockCount = 0;
    pstSpinLock->bInterruptFlag = FALSE;
    pstSpinLock->bLockFlag = FALSE;
}

/// 시스템 전역에서 사용하는 데이터를 위한 잠금 함수
void kLockForSpinLock(SPINLOCK *pstSpinLock) {
    BOOL bInterruptFlag;

    /// 인터럽트를 먼저 비활성화
    bInterruptFlag = kSetInterruptFlag(FALSE);

    /// 이미 잠겨 있다면 내가 잠갔는지 확인하고 그렇다면 잠근 횟수를 증가시킨 뒤 종료
    if (kTestAndSet(&(pstSpinLock->bLockFlag), 0, 1) == FALSE) {

        /// 자신이 잠갔다면 횟수만 증가시킴
        if (pstSpinLock->bAPICD == kGetAPICID()) {
            pstSpinLock->dwLockCount++;
            return;
        }

        /// 자신이 아닌 경우는 잠긴 것이 해제될 떄까지 대기
        while (kTestAndSet(&(pstSpinLock->bLockFlag), 0, 1) == FALSE) {
            /// kTestAndSet 함수르 계속 호출해서 메모리 버스가 Lock되는 것을 방지
            while (pstSpinLock->bLockFlag == TRUE) {
                kPause();
            }
        }
    }

    /// 잠김설정, 잠김 플래그는 위의 kTestAndSet에서 처리
    pstSpinLock->dwLockCount = 1;
    pstSpinLock->bAPICD = kGetAPICID();

    /// 인터럽트 플래그를 저장하여 unlock 수행 시 복원
    pstSpinLock->bInterruptFlag = bInterruptFlag;
}

/// 시스템 전역에서 사용하는 데이터를 위한 잠금 해제 함수
void kUnlockForSpinLock(SPINLOCK *pstSpinLock) {
    BOOL bInterruptFlag;

    /// 인터럽트를 먼저 비활성화
    bInterruptFlag = kSetInterruptFlag(FALSE);

    /// 스핀락을 잠근 태스크가 아니면 실패
    if ((pstSpinLock->bLockFlag == FALSE) || (pstSpinLock->bAPICD != kGetAPICID())) {
        kSetInterruptFlag(bInterruptFlag);
        return;
    }
    /// 스핀락을 중복으로 잠갔으면 잠긴 횟수만 감소
    if (pstSpinLock->dwLockCount > 1) {
        pstSpinLock->dwLockCount--;
        return;
    }

    /// 스핀락을 해제된 것으로 설정하고 인터럽트 플래그를 복원
    /// 인터럽트 플래그는 미리 저장해두었다가 사용
    bInterruptFlag = pstSpinLock->bInterruptFlag;
    pstSpinLock->bAPICD = 0xFF;
    pstSpinLock->dwLockCount = 0;
    pstSpinLock->bInterruptFlag = FALSE;
    pstSpinLock->bLockFlag = FALSE;

    kSetInterruptFlag(bInterruptFlag);
}