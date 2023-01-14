//
// Created by jewoo on 2023-01-14.
//
// 동기화 파일

#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"
#include "AssemblyUtility.h"

// 시스템 전역에서 사용하는 데이터를 위한 잠금 함수
BOOL kLockForSystemData(void) {
    return kSetInterruptFlag(FALSE);
}

// 시스템 전역에서 사용하는 데이터를 위한 잠금 해제 함수
void kUnlockForSystemData(BOOL bInterruptFlag) {
    kSetInterruptFlag(bInterruptFlag);
}

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
