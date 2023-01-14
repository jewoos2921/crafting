//
// Created by jewoo on 2023-01-09.
//

#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "AssemblyUtility.h"
#include "Console.h"
#include "Synchronization.h"


// 스케줄러 관련 자료구조
static SCHEDULER gs_stScheduler;
static TCB_POOL_MANAGER gs_st_TCBPoolManager;

// ============================================================================
//                      태스크 풀과 태스크 관련
// ============================================================================

// 테스크 풀 초기화
static void kInitializeTCBPool(void) {
    int i;
    kMemSet(&(gs_st_TCBPoolManager), 0, sizeof(gs_st_TCBPoolManager));

    // 태스크 풀의 어드레스를 지정하고 초기화
    gs_st_TCBPoolManager.pstStartAddress = (TCB *) TASK_TCB_POOL_ADDRESS;
    kMemSet((TCB *) TASK_TCB_POOL_ADDRESS, 0, sizeof(TCB) * TASK_MAX_COUNT);

    // TCB에 ID 할당
    for (i = 0; i < TASK_MAX_COUNT; i++) {
        gs_st_TCBPoolManager.pstStartAddress[i].stLink.qwID = i;
    }
    // TCB의 최대 개수와 할당된 횟수를 초기화
    gs_st_TCBPoolManager.iMaxCount = TASK_MAX_COUNT;
    gs_st_TCBPoolManager.iAllocatedCount = 1;
}

// TCB를 할당받음
static TCB *kAllocateTCB(void) {
    TCB *pstEmptyTCB;
    int i;

    if (gs_st_TCBPoolManager.iUseCount == gs_st_TCBPoolManager.iMaxCount) {
        return NIL;
    }

    for (i = 0; i < gs_st_TCBPoolManager.iMaxCount; i++) {
        // ID의 상위 32비트가 0이면 할당되지 않는 TCB
        if ((gs_st_TCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0) {
            pstEmptyTCB = &(gs_st_TCBPoolManager.pstStartAddress[i]);
            break;
        }
    }

    // 상위 32비트를 0이아닌 값으로 설정해서 할당된 TCB로 설정
    pstEmptyTCB->stLink.qwID = ((QWORD) gs_st_TCBPoolManager.iAllocatedCount << 32) | i;
    gs_st_TCBPoolManager.iUseCount++;
    gs_st_TCBPoolManager.iAllocatedCount++;
    if (gs_st_TCBPoolManager.iAllocatedCount == 0) {
        gs_st_TCBPoolManager.iAllocatedCount = 1;
    }

    return pstEmptyTCB;
}

// TCB를 헤제함
static void kFreeTCB(QWORD qwID) {
    int i;
    // 태스크 ID의 하위 32비트가 인데스 역할을 함
    i = GET_TCB_OFF_SET(qwID);

    // TCB 를 촉기화하고 id 설정
    kMemSet(&(gs_st_TCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));
    gs_st_TCBPoolManager.pstStartAddress[i].stLink.qwID = i;

    gs_st_TCBPoolManager.iUseCount--;
}


// 태스크를 생성
//  태스크 ID에 따라서 스택 풀에서 스택 자동 할당
TCB *kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress) {
    TCB *pstTask;
    void *pvStackAddress;
    BOOL bPreviousFlag;

    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();
    pstTask = kAllocateTCB();
    if (pstTask == NIL) {
        return NIL;
    }
    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);


    // 태스크 ID로 스택 어드레스 계산, 하위 32비트가 스택 풀의 오프셋 역할 수행
    pvStackAddress = (void *) (TASK_STACK_POOL_ADDRESS +
                               (TASK_STACK_SIZE * GET_TCB_OFF_SET(pstTask->stLink.qwID)));

    // TCB를 설정한 후 준비 리스트에 삽입하여 스케줄링될 수 있도록 함
    kSetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACK_SIZE);

    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();

    // 태스크를 준비 리스트에 삽입
    kAddTaskToReadyList(pstTask);

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);
    return pstTask;
}

// 파라미터를 이용해서 TCB를 설정
static void kSetUpTask(TCB *pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
                       void *pvStackAddress, QWORD qwStackSize) {
    // 컨텍스트 초기화
    kMemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

    // 스택에 관한 RSP, RBP 레지스터 설정
    pstTCB->stContext.vqRegister[TASK_RSP_OFFSET] = (QWORD) pvStackAddress + qwStackSize;
    pstTCB->stContext.vqRegister[TASK_RBP_OFFSET] = (QWORD) pvStackAddress + qwStackSize;

    // 세그먼트 셀레터 설정
    pstTCB->stContext.vqRegister[TASK_CS_OFFSET] = GDT_KERNEL_CODE_SEGMENT;
    pstTCB->stContext.vqRegister[TASK_DS_OFFSET] = GDT_KERNEL_DATA_SEGMENT;
    pstTCB->stContext.vqRegister[TASK_ES_OFFSET] = GDT_KERNEL_DATA_SEGMENT;
    pstTCB->stContext.vqRegister[TASK_FS_OFFSET] = GDT_KERNEL_DATA_SEGMENT;
    pstTCB->stContext.vqRegister[TASK_GS_OFFSET] = GDT_KERNEL_DATA_SEGMENT;
    pstTCB->stContext.vqRegister[TASK_SS_OFFSET] = GDT_KERNEL_DATA_SEGMENT;

    // RIP 레지스터와 인터럽트 플래그 설정
    pstTCB->stContext.vqRegister[TASK_RIP_OFFSET] = qwEntryPointAddress;

    // RFLAGS 레지스터의 IF 비트(1ㅣ트 9)를 1로 설정햐여 인터럽트 활성화
    pstTCB->stContext.vqRegister[TASK_RFLAGS_OFFSET] |= 0x0200;

    //  스택, 그리고 플래그 저장
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}



// =========================================================================================
//      스케줄러 관련
// =========================================================================================

// 스케줄러를 초기화
//  스테줄러를 초기화하는데 필요한 TCB 풀과 init 태스크도 같이 초기화
void kInitializeScheduler(void) {

    int i;

    // 태스크 풀 초기화
    kInitializeTCBPool();

    // 준비 리스트와 우선순위별 실행 횟수를 초기화하고 대기 리스트도 초기화
    for (i = 0; i < TASK_MAX_READY_LIST_COUNT; i++) {
        kInitializeList(&(gs_stScheduler.vstReadyList[i]));
        gs_stScheduler.viExecuteCount[i] = 0;
    }
    kInitializeList(&(gs_stScheduler.stWaitList));

    // TCB를 할당받아 실행 중인 태스크로 설정하여, 부팅을 수행한 태스크를 저장할 TCB를 준비
    gs_stScheduler.pstRunningTask = kAllocateTCB();
    gs_stScheduler.pstRunningTask->qwFlags = TASK_FLAGS_HIGHEST;

    // 프로세서 사용률을 계산하는데 사용하는 자료구조 초기화
    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;
}

// 현재 수행 중인 태스크를 설정
void kSetRunningTask(TCB *pstTask) {
    BOOL bPreviousFlag;

    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();
    gs_stScheduler.pstRunningTask = pstTask;

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);
}

// 현재 수행 중인 태스크를 반환
TCB *kGetRunningTask(void) {
    BOOL bPreviousFlag;
    TCB *pstRunningTask;

    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();
    pstRunningTask = gs_stScheduler.pstRunningTask;
    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);

    return pstRunningTask;
}

// 태스크 리스트에서 다음으로 실행할 태스크를 얻음
static TCB *kGetNextTaskToRun(void) {
    TCB *pstTarget = NIL;
    int iTaskCount, i, j;

    // 큐에 태스크가 있으나 모든 큐의 태스크가 1회씩 실행되는 경우 모든 큐가 프로세서를 양보해서
    // 태스크를 선택하지 못할 수 있으니 NIL일 경우 한 번 더 수행
    for (j = 0; j < 2; j++) {
        // 높은 우선순위에서 낮은 우선순위까지 리스트를 확인하여 스케줄링할 태스크를 선택
        for (i = 0; i < TASK_MAX_READY_LIST_COUNT; i++) {
            iTaskCount = kGetListCount(&(gs_stScheduler.vstReadyList[i]));
            // 만약 실행한 횟수보다 리스트의 태스크 수가 더 많음ㄴ 현재 우선순위의 태스크를 실행
            if (gs_stScheduler.viExecuteCount[i] < iTaskCount) {
                pstTarget = (TCB *) kRemoveListFromHeader(&(gs_stScheduler.vstReadyList[i]));
                gs_stScheduler.viExecuteCount[i]++;
                break;
            } else { /* 만약 실행한 횟수가 더 많으면 실행 횟수를 초기화하고 다음 우선순위로 양보함 */
                gs_stScheduler.viExecuteCount[i] = 0;
            }
        }

        // 만약 수행할 태스크를 찾았으면 종료
        if (pstTarget != NIL) { break; }
    }
    return pstTarget;
}

// 태스크을 스케줄러의 준비 리스트에 삽입
static BOOL kAddTaskToReadyList(TCB *pstTask) {
    BYTE bPriority;

    bPriority = GET_PRIORITY(pstTask->qwFlags);
    if (bPriority >= TASK_MAX_READY_LIST_COUNT) { return FALSE; }
    kAddListTotail(&(gs_stScheduler.vstReadyList[bPriority]), pstTask);
    return TRUE;
}


// 준비 큐에서 태스크를 제거
static TCB *kRemoveTaskFromReadyList(QWORD qwTaskID) {
    TCB *pstTarget;
    BYTE bPriority;

    // 태스크 ID가 유효하지 않으면 실패
    if (GET_PRIORITY(qwTaskID) >= TASK_MAX_COUNT) {
        return NIL;
    }

    // TCB 풀에서 해당 태스크의 TCB를 찾아 실제로 ID가 일치하는가 확인
    pstTarget = &(gs_st_TCBPoolManager.pstStartAddress[GET_TCB_OFF_SET(qwTaskID)]);
    if (pstTarget->stLink.qwID != qwTaskID) { return NIL; }

    // 태스크가 존재하는 준비 리스트에서 태스크 제거
    bPriority = GET_PRIORITY(pstTarget->qwFlags);

    pstTarget = kRemoveList(&(gs_stScheduler.vstReadyList[bPriority]), qwTaskID);
    return pstTarget;
}

// 태스크의 우선순위를 변경
BOOL kChangePriority(QWORD qwTaskID, BYTE bPriority) {
    TCB *pstTarget;
    BOOL bPreviousFlag;
    if (bPriority > TASK_MAX_READY_LIST_COUNT) { return FALSE; }


    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();

    // 현재 실행중인 태스크이면 우선순위만 변경
    // PIT 컨트롤러의 인터럽트(IRQ 0)가 발생하면 태스크 전환이 수행될 대 변경된 우선순위의 리스트로 이동
    pstTarget = gs_stScheduler.pstRunningTask;
    if (pstTarget->stLink.qwID == qwTaskID) {
        SET_PRIORITY(pstTarget->qwFlags, bPriority);
    } else {
        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 우선순위를 설정
        pstTarget = kRemoveTaskFromReadyList(qwTaskID);
        if (pstTarget == NIL) {
            // 태스크 ID로 직접 찾아서 설정
            pstTarget = kGetTCBInTCBPool(GET_TCB_OFF_SET(qwTaskID));
            if (pstTarget != NIL) {
                //
                SET_PRIORITY(pstTarget->qwFlags, bPriority);
            }
        } else {
            // 우선순위를 설정하고 준비 리스트에 다시 삽입
            SET_PRIORITY(pstTarget->qwFlags, bPriority);
            kAddTaskToReadyList(pstTarget);
        }
    }

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);
    return TRUE;
}

// 다른 태스크를 찾아서 전환
//          인터럽트나 예외가 발생했을 때 호출하면 안됨
void kSchedule(void) {
    TCB *pstRunningTask;
    TCB *pstNextTask;
    BOOL bPreviousFlag;

    // 전환할 태스크가 있어야 함
    if (kGetReadyTaskCount() < 1) { return; }

    // 전환하는 도중 인터럽트가 발생하여 태스크 전환이 또 일어나면 곤란하므로
    // 전환하는 도중 인터럽트가 발생지 못하도록 설정
    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();

    // 실행할 다음 태스크를 얻음
    pstNextTask = kGetNextTaskToRun();
    if (pstNextTask == NIL) {
        // 임계 영역 끝
        kUnlockForSystemData(bPreviousFlag);
        return;

    }

    // 현재 수행중인 태스크의 정보를 수정한 뒤 콘텍스트 전환
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;


    // 유휴 태스크에서 전환되었다면 사용한 프로세서 시간을 증가
    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE) {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSOR_TIME - gs_stScheduler.iProcessorTime;
    }

    // 태스크 종료 플래그가 설정된 경우 콘택스트를 저장할 필요가 없으므로 ,
    // 대기 리스트에 삽입하고 콘텍스트 전환
    if (pstNextTask->qwFlags & TASK_FLAGS_END_TASK) {
        kAddListTotail(&(gs_stScheduler.stWaitList), pstRunningTask);
        kSwitchContext(NIL, &(pstNextTask->stContext));
    } else {
        kAddTaskToReadyList(pstRunningTask);
        kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
    }


    // 프로세서 사용시간을 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSOR_TIME;
    kSetInterruptFlag(bPreviousFlag);

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);
}

// 인터럽트가 발생했을 때 다른 태스크를 찾아 전환
//  반드시 인터럽트나 예외가 발생했을 때 호출해야 함
BOOL kScheduleInterrupt(void) {
    TCB *pstRunningTask;
    TCB *pstNextTask;
    BOOL bPreviousFlag;

    char *pcContextAddress;
    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();
    // 전환할 태스크가 업으면 종료
    pstNextTask = kGetNextTaskToRun();
    if (pstNextTask == NIL) {
        // 임계 영역 끝
        kUnlockForSystemData(bPreviousFlag);
        return FALSE;
    }

    //====================================================================
    // 태스크 전환 처리
    //      인터럽트 핸들러에서 저장한 콘텍스트를 다른 콘텍스트로 덮어쓰는 방법으로 처리
    //====================================================================
    pcContextAddress = (char *) IST_START_ADDRESS + IST_SIZE - sizeof(CONTEXT);

    // 현재 수행 중인 태스크의 정보를 수정한 뒤 콘텍스트 전환
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;


    // 유휴 태스크에서 전환되었다면 사용한 프로세서 시간을 증가
    if ((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE) {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSOR_TIME;
    }

    // 태스크 종료 플래그사 설정된 경우 콘텍스트를 저장하지 않고 대기 리스트에만 삽입
    if (pstRunningTask->qwFlags & TASK_FLAGS_END_TASK) {
        kAddListTotail(&(gs_stScheduler.stWaitList), pstRunningTask);
    }
        // 태스크가 종료되지 않으면 IST에 있는 콘텍스트를 복사하고, 현재 태스크를 준비 리스트로 옮김
    else {
        kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
        kAddTaskToReadyList(pstRunningTask);
    }

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);


    // 전환해서 실행할 태스크를 Running Task로 설정하고 콘텍스트를 IST에 복사해서
    // 자동으로 태스크 전환이 일어나도록 함
    kMemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(CONTEXT));
    // 프로세서 사용 시간을 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSOR_TIME;
    return TRUE;
}

// 프로세서를 사용할 수 있는 시간을 하나 줄임
void kDecreaseProcessorTime(void) {
    if (gs_stScheduler.iProcessorTime > 0) {
        gs_stScheduler.iProcessorTime--;
    }
}

// 프로세서를 사용할 수 있는 시간이 다 되었는지 여부를 반환
BOOL kIsProcessorTimeExpired(void) {
    if (gs_stScheduler.iProcessorTime <= 0) {
        return TRUE;
    }
    return FALSE;
}

// 태스크를 종료
BOOL kEndTask(QWORD qwTaskID) {
    TCB *pstTarget;
    BYTE bPriority;
    BOOL bPreviousFlag;


    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();

    // 현재 실행 중인 태스크면 EndTask 비트를 설정하고 태스크를 전환
    pstTarget = gs_stScheduler.pstRunningTask;
    if (pstTarget->stLink.qwID == qwTaskID) {
        pstTarget->qwFlags |= TASK_FLAGS_END_TASK;
        SET_PRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);

        // 임계 영역 끝
        kUnlockForSystemData(bPreviousFlag);

        kSchedule();

        // 태스크가 전환되었으므로 아래 코드는 절대 실행되지 않음
        while (1);
    } else { // 실행중인 태스크가 아니면 준비 큐에 직접 찾아서 대기 리스트에 연결

        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 태스크 종료 비트 설정
        pstTarget = kRemoveTaskFromReadyList(qwTaskID);
        if (pstTarget == NIL) {

            // 태스크 ID로 직접 찾아서 설정
            pstTarget = kGetTCBInTCBPool(GET_TCB_OFF_SET(qwTaskID));
            if (pstTarget != NIL) {
                pstTarget->qwFlags |= TASK_FLAGS_END_TASK;
                SET_PRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
            }
            // 임계 영역 끝
            kUnlockForSystemData(bPreviousFlag);
            return TRUE;
        }

        pstTarget->qwFlags |= TASK_FLAGS_END_TASK;
        SET_PRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
        kAddListTotail(&(gs_stScheduler.stWaitList), pstTarget);
    }

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);
    return TRUE;
}

// 태스크가 자신을 종료함
void kExitTask(void) {
    kEndTask(gs_stScheduler.pstRunningTask->stLink.qwID);
}

// 준비 큐에 있는 모든 태스크의 수를 반환
int kGetReadyTaskCount(void) {
    int i;
    int iTotalCount = 0;
    BOOL bPreviousFlag;

    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();

    // 모든 준비 큐를 확인하여 태스크 개수를 구함
    for (i = 0; i < TASK_MAX_READY_LIST_COUNT; i++) {
        iTotalCount += kGetListCount(&(gs_stScheduler.vstReadyList[i]));
    }

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);
    return iTotalCount;
}

// 전체 태스크의 수를 반환
int kGetTaskCount(void) {
    int iTotalCount;
    BOOL bPreviousFlag;


    // 준비 큐의 태스크 수를 구한 후 대기 큐의 태스크 수와 현재 수행 중인 태스크 수를 더함
    iTotalCount = kGetReadyTaskCount();

    // 임계 영역 시작
    bPreviousFlag = kLockForSystemData();

    iTotalCount += kGetListCount(&(gs_stScheduler.stWaitList)) + 1;

    // 임계 영역 끝
    kUnlockForSystemData(bPreviousFlag);

    return iTotalCount;
}

// TCB 풀에서 해당 오프셋의 TCB를 반환
TCB *kGetTCBInTCBPool(int iOffset) {
    if ((iOffset < -1) && (iOffset > TASK_MAX_COUNT)) {
        return NIL;
    }
    return &gs_st_TCBPoolManager.pstStartAddress[iOffset];
}


// 프로세서의 사용률 반환
QWORD kGetProcessorLoad(void) {
    return gs_stScheduler.qwProcessorLoad;
}

// =========================================================================================
//      유휴 태스크 관련
// =========================================================================================

// 유휴 태스크
//      대기 큐에 삭제 대기 중인 태스크를 정리
void kIdleTask(void) {
    TCB *pstTask;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickIdleTask;
    QWORD qwTaskID;
    BOOL bPreviousFlag;

    // 프로세서 사용량 계산을 위해 기준 정보를 저장
    qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();

    while (1) {
        // 현재 상태를 저장
        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

        // 프로세서 사용량을 계산
        // 100 - (유휴 태스크가 사용한 프로세서 시간) * 100 / (시스템 전체에서 사용한 프로세서 시간)
        if (qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0) {
            gs_stScheduler.qwProcessorLoad = 0;
        } else {
            gs_stScheduler.qwProcessorLoad = 100 - (qwCurrentSpendTickIdleTask - qwLastSpendTickInIdleTask) *
                                                   100 / (qwCurrentMeasureTickCount - qwLastMeasureTickCount);
        }

        // 현재 상태를 이전 상태에 보관
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickIdleTask;

        // 프로세서의 부하에 따라 쉬게 함
        kHaltProcessorByLoad();

        // 대기 큐에 대기 중인 태스크가 있으면 태스크를 종료함
        if (kGetListCount(&(gs_stScheduler.stWaitList)) >= 0) {
            while (1) {

                // 임계 영역 시작
                bPreviousFlag = kLockForSystemData();
                pstTask = kRemoveListFromHeader(&(gs_stScheduler.stWaitList));
                if (pstTask == NIL) {
                    // 임계 영역 끝
                    kUnlockForSystemData(bPreviousFlag);
                    break;
                }

                qwTaskID = pstTask->stLink.qwID;
                kFreeTCB(pstTask->stLink.qwID);

                // 임계 영역 끝
                kUnlockForSystemData(bPreviousFlag);

                kPrintf("IDLE: Task ID[0x%q] is completely ended.\n",
                        qwTaskID);
            }
        }
        kSchedule();
    }
}

// 측정된 프로세서 부하에 따라 프로세서를 쉬게 함
void kHaltProcessorByLoad(void) {
    if (gs_stScheduler.qwProcessorLoad < 40) {
        kHlt();
        kHlt();
        kHlt();
    } else if (gs_stScheduler.qwProcessorLoad < 80) {
        kHlt();
        kHlt();
    } else if (gs_stScheduler.qwProcessorLoad < 95) {
        kHlt();
    }
}
