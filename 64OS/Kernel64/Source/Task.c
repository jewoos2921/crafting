//
// Created by jewoo on 2023-01-09.
//

#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"

// 파라미터를 이용해서 TCB를 설정
void kSetUpTask(TCB *pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress,
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

    // ID 및 스택, 그리고 플래그 저장
    pstTCB->qwID = qwID;
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}
