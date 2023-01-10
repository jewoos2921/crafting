//
// Created by jewoo on 2023-01-09.
//

#ifndef CRAFTING_TASK_H
#define CRAFTING_TASK_H

#include "Types.h"


// SS, RSP, RFLAGS, CS, RIP + ISR에서 저장하는 19개의 레지스터
#define TASK_REGISTER_COUNT         (5 + 19)
#define TASK_REGISTER_SIZE          8

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
#define TASK_RBP_OFFSET               18
#define TASK_RIP_OFFSET              19
#define TASK_CS_OFFSET               20
#define TASK_RFLAGS_OFFSET           21
#define TASK_RSP_OFFSET              22
#define TASK_SS_OFFSET               23


// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// 콘텍스트에 관련된 자료구조
typedef struct kContextStruct {
    QWORD vqRegister[TASK_REGISTER_COUNT];
} CONTEXT;

// 테스크의 상태를 관리하는 자료구조
typedef struct kTaskControlBlockStruct {
    // 컨텍스트
    CONTEXT stContext;

    // ID 및 츨래그
    QWORD qwID;
    QWORD qwFlags;

    // 스택의 어드레스와 크기
    void *pvStackAddress;
    QWORD qwStackSize;
} TCB;
#pragma pack(pop)

void kSetUpTask(TCB *pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress,
                void *pvStackAddress, QWORD qwStackSize);


#endif //CRAFTING_TASK_H
