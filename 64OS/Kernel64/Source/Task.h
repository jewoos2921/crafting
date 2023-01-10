//
// Created by jewoo on 2023-01-09.
//

#ifndef CRAFTING_TASK_H
#define CRAFTING_TASK_H

#include "Types.h"
#include "List.h"


// SS, RSP, RFLAGS, CS, RIP + ISR에서 저장하는 19개의 레지스터
#define TASK_REGISTER_COUNT         (5 + 19)
#define TASK_REGISTER_SIZE           8

// Context 자료구조의 레지스터 오프셋
#define TASK_GS_OFFSET               0
#define TASK_FS_OFFSET               1
#define TASK_ES_OFFSET               2
#define TASK_DS_OFFSET               3
#define TASK_R15_OFFSET              4
#define TASK_R14_OFFSET              5
#define TASK_R13_OFFSET              6
#define TASK_R12_OFFSET              7
#define TASK_R11_OFFSET              8
#define TASK_R10_OFFSET              9
#define TASK_R9_OFFSET               10
#define TASK_R8_OFFSET               11
#define TASK_RSI_OFFSET              12
#define TASK_RDI_OFFSET              13
#define TASK_RDX_OFFSET              14
#define TASK_RCX_OFFSET              15
#define TASK_RBX_OFFSET              16
#define TASK_RAX_OFFSET              17
#define TASK_RBP_OFFSET              18
#define TASK_RIP_OFFSET              19
#define TASK_CS_OFFSET               20
#define TASK_RFLAGS_OFFSET           21
#define TASK_RSP_OFFSET              22
#define TASK_SS_OFFSET               23


// 태스크 풀의 어드레스
#define TASK_TCB_POOL_ADDRESS           0x800000
#define TASK_MAX_COUNT                  1024

// 스택 풀과 스택의 크기
#define TASK_STACK_POOL_ADDRESS         (TASK_TCB_POOL_ADDRESS + sizeof(TCB) * TASK_MAX_COUNT)
#define TASK_STACK_SIZE                 8192

// 유효하지 않은 태스크 ID
#define TASK_INVALID_ID                 0xFFFFFFFFFFFFFFFF

// 태스크가 최대로 쓸 수 있는 프로세스 시간(ms)
#define TASK_PROCESSOR_TIME             5

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// 콘텍스트에 관련된 자료구조
typedef struct kContextStruct {
    QWORD vqRegister[TASK_REGISTER_COUNT];
} CONTEXT;

// 테스크의 상태를 관리하는 자료구조
typedef struct kTaskControlBlockStruct {
    // 다음 데이터의 위치와 ID
    LIST_LINK stLink;

    // 플래그
    QWORD qwFlags;

    // 컨텍스트
    CONTEXT stContext;


    // 스택의 어드레스와 크기
    void *pvStackAddress;
    QWORD qwStackSize;
} TCB;

// TCB 풀의 상태를 관리하는 자료 구조
typedef struct kTCBPoolManagerStruct {
    // 태스크 풀의 대한 정보
    TCB *pstStartAddress;
    int iMaxCount;
    int iUseCount;

    // TCB가 할당된 횟수
    int iAllocatedCount;
} TCB_POOL_MANAGER;

// 스케줄러의 상태를 관리하는 자료구조
typedef struct kSchedulerStruct {
    // 현재 수행 중인 태스크
    TCB *pstrRunning;
    // 현재 수행중인 태스크가 사용할 수 있는 프로세서 시간
    int iProcessorTime;

    // 실행할 태스크가 준비 중인 리스틑
    LIST stReadyList;
} SCHEDULER;

#pragma pack(pop)

// 함수
// =========================================================================================
//      태스크 폴과 태스크 관련
// =========================================================================================
void kInitializeTCBPool(void);

TCB *kAllocateTCB(void);

void kFreeTCB(QWORD qwID);

TCB *kCreateTask(QWORD qwFlags, QWORD qwEntryPointAddress);

void kSetUpTask(TCB *pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress,
                void *pvStackAddress, QWORD qwStackSize);

// =========================================================================================
//      스케줄러 관련
// =========================================================================================
void kInitializeScheduler(void);

void kSetRunningTask(TCB *pstTask);

TCB *kGetRunningTask(void);

TCB *kGetNextTaskToRun(void);

void kAddTaskToReadyList(TCB *pstTask);

void kSchedule(void);

BOOL kScheduleInterrupt(void);

BOOL kDecreaseProcessorTime(void);

BOOL kIsProcessorTimeExpired(void);

#endif //CRAFTING_TASK_H
